#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define HISTORY_SIZE 5

void print_history(char *history[HISTORY_SIZE], size_t next_index);
void store_input(char *history[HISTORY_SIZE], size_t *next_index_ptr,
                 const char *input_line);
void free_history(char *history[HISTORY_SIZE]);
int main(void) {
  char *history[HISTORY_SIZE];
  size_t next_index = 0;
  char *line = NULL;
  size_t len = 0;
  ssize_t read_bytes;
  for (int i = 0; i < HISTORY_SIZE; i++) {
    history[i] = NULL;
  }

  printf("history program starting\n");
  while (1) {
    printf("enter input: ");
    if (read_bytes == -1) {
      break;
    }
    if (read_bytes > 0 && line[read_bytes - 1] == '\n') {
      line[read_bytes - 1] = '\0';
    }
    if (strcmp(line, "print") == 0) {
      store_input(history, &next_index, line);
      print_history(history, next_index);
    } else {
      store_input(history, &next_index, line);
    }
    line = NULL;
    len = 0;
  }
  printf("\n program exited \n");
  free_history(history);
  if (line != NULL) {
    free(line);
  }
  return 0;
}

void store_input(char *history[HISTORY_SIZE], size_t *next_index_ptr,
                 const char *input_line) {
  size_t index = *next_index_ptr;

  if (history[index] != NULL) {
    free(history[index]);
  }

  history[index] = strdup(input_line);
  if (history[index] == NULL) {
    printf("strdup failed");
    exit(EXIT_FAILURE);
  }
  *next_index_ptr = (index + 1) % HISTORY_SIZE;
}

void print_history(char *history[HISTORY_SIZE], size_t next_index) {
  size_t start_index = 0;
  int i;
  for (i = 0; i < HISTORY_SIZE; i++) {

    size_t print_index = (start_index + i) % HISTORY_SIZE;
    if (history[print_index] != NULL) {
      printf("%s\n", history[print_index]);
    }
  }
}

void free_history(char *history[HISTORY_SIZE]) {
  for (int i = 0; i < HISTORY_SIZE; i++) {
    if (history[i] != NULL) {
      free(history[i]);
      history[i] = NULL;
    }
  }
}
