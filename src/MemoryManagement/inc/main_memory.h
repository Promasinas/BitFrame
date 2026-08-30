#ifndef __MAIN_MEMORY_H__
#define __MAIN_MEMORY_H__

#include <stdbool.h>
#include <stddef.h>

#define BLOCK_LIST_INIT_SIZE 128

typedef struct {
    size_t block_size;
    void*  addr;
} memory_block_t;

void  clear_blocks(void);
void  clear_main_memory(void);
bool  add_block(size_t block_size);
bool  activate_blocks(void);
memory_block_t get_block_by_index(size_t block_index);
size_t get_top_block_index(void);

#endif
