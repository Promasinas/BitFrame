#ifndef __MAIN_MEMORY_H__
#define __MAIN_MEMORY_H__

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    size_t block_size;
    size_t addr_offset;
}memory_block_t;

size_t* main_memory_ptr;
size_t main_memory_addr_size;

memory_block_t* block_list;
size_t block_list_size;
size_t block_counter;

bool clear_main_memory(void);

bool activate_blocks(void);
bool write_to_block(size_t block_index, size_t block_addr_offset, void* data, size_t data_addr_size);
const size_t* get_block_ptr(size_t block_index, size_t block_addr_offset);

#endif
