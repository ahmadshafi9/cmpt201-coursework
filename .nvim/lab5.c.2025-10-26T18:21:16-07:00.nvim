#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

struct header {
  uint64_t size;
  struct header *next;
  int id;
};

// Initialize a memory block
void initialize_block(struct header *block, uint64_t size, struct header *next,
                      int id) {
  block->size = size;
  block->next = next;
  block->id = id;
}

// First-Fit: find the first block that is large enough
int find_first_fit(struct header *free_list_ptr, uint64_t size) {
  struct header *current = free_list_ptr;
  while (current != NULL) {
    if (current->size >= size) {
      return current->id;
    }
    current = current->next;
  }
  return -1; // Not found
}

// Best-Fit: find the smallest block that fits
int find_best_fit(struct header *free_list_ptr, uint64_t size) {
  struct header *current = free_list_ptr;
  int best_fit_id = -1;
  uint64_t min_size_diff = UINT64_MAX;

  while (current != NULL) {
    if (current->size >= size && (current->size - size) < min_size_diff) {
      min_size_diff = current->size - size;
      best_fit_id = current->id;
    }
    current = current->next;
  }
  return best_fit_id;
}

// Worst-Fit: find the largest block available
int find_worst_fit(struct header *free_list_ptr, uint64_t size) {
  struct header *current = free_list_ptr;
  int worst_fit_id = -1;
  uint64_t max_size = 0;

  while (current != NULL) {
    if (current->size >= size && current->size > max_size) {
      max_size = current->size;
      worst_fit_id = current->id;
    }
    current = current->next;
  }
  return worst_fit_id;
}

int main(void) {

  // Allocate memory blocks
  struct header *free_block1 = (struct header *)malloc(sizeof(struct header));
  struct header *free_block2 = (struct header *)malloc(sizeof(struct header));
  struct header *free_block3 = (struct header *)malloc(sizeof(struct header));
  struct header *free_block4 = (struct header *)malloc(sizeof(struct header));
  struct header *free_block5 = (struct header *)malloc(sizeof(struct header));

  // Initialize the blocks with sizes and IDs
  initialize_block(free_block1, 6, free_block2, 1);
  initialize_block(free_block2, 12, free_block3, 2);
  initialize_block(free_block3, 24, free_block4, 3);
  initialize_block(free_block4, 8, free_block5, 4);
  initialize_block(free_block5, 4, NULL, 5);

  struct header *free_list_ptr = free_block1;

  // Test memory allocation algorithms with requested size = 7
  int first_fit_id = find_first_fit(free_list_ptr, 7);
  int best_fit_id = find_best_fit(free_list_ptr, 7);
  int worst_fit_id = find_worst_fit(free_list_ptr, 7);

  // Print the results
  printf("The ID for First-Fit algorithm is: %d\n", first_fit_id);
  printf("The ID for Best-Fit algorithm is: %d\n", best_fit_id);
  printf("The ID for Worst-Fit algorithm is: %d\n", worst_fit_id);

  // Free allocated memory
  free(free_block1);
  free(free_block2);
  free(free_block3);
  free(free_block4);
  free(free_block5);

  return 0;
}

/*
First start at the first block in your free list (the head).

While there is a next block in the list:
   Check if the current block and the next block are physically next to each
other in memory. This means the end of the current block touches the start of
the next block. If they are contiguous: Merge them into one bigger block by
adding the next block's size to the current block's size. Update the 'next'
pointer of the current block to skip over the next block. If they are not
contiguous: Move to the next block and repeat the check.

Keep doing this until you reach the end of the list.

Result:
All adjacent free blocks are merged.
Memory fragmentation is reduced.
The free list is updated with bigger, coalesced blocks ready for future
allocation.
*/
