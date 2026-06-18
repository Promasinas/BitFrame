#ifndef __MAIN_MEMORY_H__
#define __MAIN_MEMORY_H__

#include <stdint.h>
#include <stdbool.h>

extern uint64_t* main_memory_block;
extern uint64_t main_memory_block_size;

//以64bit为最小单位存储
bool set_main_memory_block_size(uint64_t byte_size);
bool clear_main_memory_block(void);

#endif
