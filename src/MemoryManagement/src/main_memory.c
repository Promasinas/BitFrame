#include "main_memory.h"

uint64_t* main_memory_block = nullptr;
uint64_t main_memory_block_size = 0;

bool set_main_memory_block_size(uint64_t byte_size){
    if(byte_size%8 != 0){
        return false;
    }

    main_memory_block_size = byte_size >> 3; //以64bit为最小单位存储
    main_memory_block = new uint64_t[main_memory_block_size];
    
    return true;
}

bool clear_main_memory_block(){
    if(main_memory_block == nullptr){
        return false;
    }

    delete[] main_memory_block;
    return true;
}

