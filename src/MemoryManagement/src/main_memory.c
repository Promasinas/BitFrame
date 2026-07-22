#include "main_memory.h"
#include "log.h"
#include <stdlib.h>

void* main_memory = NULL;

memory_block_t* block_list = NULL;
size_t block_count = 0;
size_t block_list_capacity = 0;

void clear_memory(void){
    free(main_memory);
    main_memory = NULL;

    free(block_list);
    block_list = NULL;

    block_count = 0;
    block_list_capacity = 0;
}

bool add_block(size_t block_size){
    if(block_list_capacity == 0){
        void* temp = malloc(sizeof(memory_block_t)*LIST_UNIT_SIZE);
        if(temp == NULL){
            //log
            return false;
        }

        block_list = temp;
        block_list_capacity = LIST_UNIT_SIZE;
    }

    if(block_count >= block_list_capacity){
        void* temp = realloc(block_list, sizeof(memory_block_t)*(LIST_UNIT_SIZE+block_list_capacity));
        if(temp == NULL){
            //log 
            return false;
        }

        block_list = temp;
        block_list_capacity += LIST_UNIT_SIZE;
    }

    block_list[block_count].block_size = block_size;
    block_count++;
    return true;
}

bool activate_blocks(void){
    
}

void* get_block_ptr_by_index(size_t index){
    
}
