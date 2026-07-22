#ifndef __MAIN_MEMORY_H__
#define __MAIN_MEMORY_H__

#define LIST_UNIT_SIZE 16

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    size_t block_size;
    size_t addr;
}memory_block_t;

void* main_memory;

memory_block_t* block_list;
size_t block_count;
size_t block_list_capacity;

void clear_memory(void);
bool add_block(size_t block_size);
bool activate_blocks(void);
void* get_block_ptr_by_index(size_t index);

#endif
