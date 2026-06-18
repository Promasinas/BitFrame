#include "main_memory.h"
#include "log.h"
#include <stdlib.h>

uint64_t* main_memory_block = NULL;
uint64_t main_memory_block_size = 0;

bool set_main_memory_block_size(uint64_t byte_size){
    if(byte_size % 8 != 0){
        log_error("set_main_memory_block_size: byte_size (%lu) is not a multiple of 8", byte_size);
        return false;
    }

    main_memory_block_size = byte_size >> 3; //以64bit为最小单位存储
    main_memory_block = (uint64_t*)malloc(main_memory_block_size * sizeof(uint64_t));
    if(main_memory_block == NULL){
        log_error("set_main_memory_block_size: malloc failed for %lu blocks", main_memory_block_size);
        return false;
    }
    log_info("set_main_memory_block_size: allocated %lu x 64-bit blocks (%lu bytes)",
             main_memory_block_size, byte_size);

    return true;
}

bool clear_main_memory_block(void){
    if(main_memory_block == NULL){
        log_error("clear_main_memory_block: main_memory_block is already null");
        return false;
    }

    free(main_memory_block);
    main_memory_block = NULL;
    log_info("clear_main_memory_block: memory released");
    return true;
}
