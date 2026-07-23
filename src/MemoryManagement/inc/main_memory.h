#ifndef __MAIN_MEMORY_H__
#define __MAIN_MEMORY_H__

#define BLOCK_LIST_UNIT_SIZE 128

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    size_t block_size;
    void* addr;
}memory_block_t;

void clear_blocks();
void clear_main_memory();
bool add_block(size_t block_size);
bool activate_blocks();
// bool allocate_blocks();
void* get_block_by_index(size_t block_index);

#endif
