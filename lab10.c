/*
1. How is the client sending data to the server? What protocol?
   The client is sending data using the standard **TCP (Transmission Control Protocol)**.
   The data is sent using the socket function `send()` or `write()`, which operates on a reliable, stream-based connection.

2. What data is the client sending to the server?
   The client is sending **text strings/messages** (e.g., "Hello", "Apple", "Car", "Green", "Dog"). 
   Each client sends a total of NUM_MSG_PER_CLIENT (5) messages.

Understanding the Server:
1. Explain the argument that the `run_acceptor` thread is passed as an argument.
   The `run_acceptor` thread is passed a pointer to a **structure** (e.g., `server_info_t`) which contains the **listening socket file descriptor** (`listen_fd`) and a **pointer to the array of client structures** (`client_array`) so it can store information about new connections and the thread handles.

2. How are received messages stored?
   Received messages are stored in a **linked list** of nodes (e.g., `struct msg_node`). Each node holds the message data and a pointer to the next node. The list is managed by a **global head pointer** (`msg_list_head`).

3. What does `main()` do with the received messages?
   The `main()` function **waits** until the required number of messages (`MAX_CLIENTS * NUM_MSG_PER_CLIENT`) has been collected. Once collected, it iterates over the linked list, **prints** all the collected messages, **prints the total count**, and then cleans up the server resources.

4. How are threads used in this sample code?
   - One **Acceptor Thread (`run_acceptor`)** handles new client connections by continuously calling `accept()`.
   - For each connected client, a new **Client Thread (`run_client`)** is launched to handle all communication with that specific client (i.e., receiving its messages and adding them to the global list).

Explain the use of non-blocking sockets in this lab.
How are sockets made non-blocking?
   Sockets are made non-blocking using the `fcntl()` function to set the `O_NONBLOCK` flag: `fcntl(socket_fd, F_SETFL, fcntl(socket_fd, F_GETFL, 0) | O_NONBLOCK);`.

What sockets are made non-blocking?
   The **listening socket** (`listen_fd`) is made non-blocking. The client sockets are typically left blocking.

Why are these sockets made non-blocking? What purpose does it serve?
   The listening socket is made non-blocking so that the `run_acceptor` thread **does not block indefinitely** on `accept()`. If `accept()` returns `EWOULDBLOCK` or `EAGAIN`, the thread can immediately check its **shared stop flag** (e.g., `acceptor_info->run`). This allows the server to cleanly and promptly **shut down** when signaled by the `main()` function, rather than waiting for another client to connect.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define NUM_MSG_PER_CLIENT 5
#define MAX_MSG_LEN 128
#define MAX_CLIENTS 3
#define BACKLOG 10
#define MAX_BUFFER 256

const char *messages[NUM_MSG_PER_CLIENT] = {
    "Hello",
    "Apple",
    "Car",
    "Green",
    "Dog"
};

struct msg_node {
    char message[MAX_MSG_LEN];
    struct msg_node *next;
};

typedef struct {
    pthread_t thread;
    int socket_fd;
    int client_id;
    int run;
    int is_active;
} client_info_t;

typedef struct {
    int listen_fd;
    int run;
    client_info_t *client_array;
} acceptor_info_t;

struct msg_node *msg_list_head = NULL;
pthread_mutex_t message_list_mutex = PTHREAD_MUTEX_INITIALIZER;
int num_messages_received = 0;
int num_clients = 0;

void add_to_list(struct msg_node **head, const char *msg) {
    struct msg_node *new_node = (struct msg_node *)malloc(sizeof(struct msg_node));
    if (new_node == NULL) {
        perror("malloc failed");
        return;
    }
    strncpy(new_node->message, msg, MAX_MSG_LEN - 1);
    new_node->message[MAX_MSG_LEN - 1] = '\0';
    new_node->next = *head;
    *head = new_node;
}

void *run_client_starter(void *arg) {
    int client_id = *(int *)arg;
    int client_fd;
    struct sockaddr_in serv_addr;

    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Client: Socket creation error");
        return NULL;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        perror("Client: Invalid address/ Address not supported");
        close(client_fd);
        return NULL;
    }

    if (connect(client_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Client: Connection Failed");
        close(client_fd);
        return NULL;
    }

    for (int i = 0; i < NUM_MSG_PER_CLIENT; i++) {
        char buffer[MAX_MSG_LEN];
        snprintf(buffer, MAX_MSG_LEN, "[Client %d] %s", client_id, messages[i]);

        ssize_t bytes_sent = send(client_fd, buffer, strlen(buffer) + 1, 0);
        if (bytes_sent > 0) {
            printf("Sent: %s\n", messages[i]);
        } else {
            perror("Client: Send failed");
            break;
        }
        usleep(10000);
    }

    close(client_fd);
    
    return NULL;
}

void *run_client(void *arg) {
    client_info_t *client_info = (client_info_t *)arg;
    int client_fd = client_info->socket_fd;
    char buffer[MAX_BUFFER];
    ssize_t bytes_read;

    client_info->run = 1;

    while (client_info->run) {
        bytes_read = recv(client_fd, buffer, MAX_BUFFER - 1, 0);

        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            printf("Collected: %s\n", buffer);
            
            pthread_mutex_lock(&message_list_mutex);
            
            add_to_list(&msg_list_head, buffer);
            
            num_messages_received++;
            
            pthread_mutex_unlock(&message_list_mutex);
            
        } else if (bytes_read == 0) {
            printf("Client disconnected gracefully.\n");
            client_info->run = 0;
        } else {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                 perror("recv failed");
            }
            client_info->run = 0;
        }
    }
    
    client_info->is_active = 0;
    return NULL;
}

void *run_acceptor(void *arg) {
    acceptor_info_t *info = (acceptor_info_t *)arg;
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);
    int client_fd;
    int client_id_counter = 1;
    
    info->run = 1;

    while (info->run) {
        client_fd = accept(info->listen_fd, (struct sockaddr *)&client_addr, &addrlen);
        
        if (client_fd >= 0) {
            
            if (num_clients < MAX_CLIENTS) {
                printf("Client connected!\n");
                
                client_info_t *current_client_info = &info->client_array[num_clients]; 

                current_client_info->socket_fd = client_fd;
                current_client_info->client_id = client_id_counter++;
                current_client_info->is_active = 1;
                
                int result = pthread_create(
                    &current_client_info->thread, 
                    NULL, 
                    run_client, 
                    (void *)current_client_info
                );
                
                if (result != 0) {
                    perror("pthread_create failed");
                    close(client_fd);
                    current_client_info->is_active = 0;
                } else {
                    num_clients++; 
                }

            } else {
                printf("Not accepting any more clients!\n");
                close(client_fd);
            }
            
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                usleep(50000);
                continue;
            } else if (errno == EINTR) {
                continue;
            } else {
                if (info->run) {
                    perror("accept failed unexpectedly");
                }
                break;
            }
        }
        
        if (num_clients >= MAX_CLIENTS) {
             info->run = 0; 
        }
    }
    
    printf("Acceptor thread finished accepting clients. Cleaning up...\n");

    for (int i = 0; i < MAX_CLIENTS; i++) {
        client_info_t *client = &info->client_array[i];
        
        if (client->is_active) {
            client->run = 0; 
            pthread_join(client->thread, NULL); 
            close(client->socket_fd);
        }
    }
    
    printf("All client threads cleaned up.\n");
    return NULL;
}

int set_socket_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl F_GETFL failed");
        return -1;
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl F_SETFL O_NONBLOCK failed");
        return -1;
    }
    return 0;
}

int main(void) {
    int listen_fd;
    struct sockaddr_in serv_addr;
    pthread_t acceptor_thread;
    client_info_t client_array[MAX_CLIENTS];
    acceptor_info_t acceptor_info;

    memset(client_array, 0, sizeof(client_array));

    if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Server: Socket creation error");
        return 1;
    }

    int opt = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        return 1;
    }

    if (set_socket_nonblocking(listen_fd) < 0) {
        close(listen_fd);
        return 1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    if (bind(listen_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Server: Bind failed");
        close(listen_fd);
        return 1;
    }

    if (listen(listen_fd, BACKLOG) < 0) {
        perror("Server: Listen failed");
        close(listen_fd);
        return 1;
    }

    acceptor_info.listen_fd = listen_fd;
    acceptor_info.client_array = client_array;
    acceptor_info.run = 1;

    if (pthread_create(&acceptor_thread, NULL, run_acceptor, (void *)&acceptor_info) != 0) {
        perror("pthread_create failed for acceptor");
        close(listen_fd);
        return 1;
    }
    
    int target_messages = MAX_CLIENTS * NUM_MSG_PER_CLIENT;
    printf("Server listening on port %d. Waiting for %d messages...\n", PORT, target_messages);

    while (1) {
        int current_count;

        pthread_mutex_lock(&message_list_mutex);
        current_count = num_messages_received;
        pthread_mutex_unlock(&message_list_mutex);

        if (current_count >= target_messages) {
            break;
        }

        usleep(10000);
    }

    acceptor_info.run = 0;
    
    pthread_join(acceptor_thread, NULL);
    
    close(listen_fd);

    struct msg_node *current = msg_list_head;
    struct msg_node *next_node;
    int count = 0;
    
    while (current != NULL) {
        printf("Collected: %s\n", current->message);
        next_node = current->next;
        free(current);
        current = next_node;
        count++;
    }

    printf("Collected: %d\n", count);

    if (count >= target_messages) {
        printf("All messages were collected!\n");
    }

    return 0;
}
