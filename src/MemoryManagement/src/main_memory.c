#include "main_memory.h"
#include <stdlib.h>
#include <stdint.h>
#include "log.h"

static void* main_memory = NULL;
static memory_block_t* block_list = NULL;
static size_t block_counter = 0;
static size_t block_list_capacity = 0;
static bool blocks_activated = false;

void clear_main_memory(){
    free(main_memory);
    main_memory = NULL;
}

void clear_blocks(){
    free(block_list);
    block_list = NULL;
    block_counter = 0;
    block_list_capacity = 0;
    blocks_activated = false;
}

bool add_block(size_t block_size){
    if(block_counter == 0){
        block_list_capacity = BLOCK_LIST_INIT_SIZE;
        void* temp = malloc(sizeof(memory_block_t)*block_list_capacity);
        if(temp == NULL){
            log_error("Failed to allocate memory for block list");
            return false;
        }
        block_list = temp;
    }

    if(block_counter >= block_list_capacity){
        block_list_capacity += BLOCK_LIST_INIT_SIZE;
        void* temp = realloc(block_list, sizeof(memory_block_t)*block_list_capacity);
        if(temp == NULL){
            log_error("Failed to allocate memory for block list");
            return false;
        }
        block_list = temp;
    }

    block_list[block_counter].block_size = block_size;
    block_list[block_counter].addr = NULL;
    block_counter++;
    return true;
}

size_t get_top_block_index(){
    return block_counter - 1;
}

bool activate_blocks(){
    if(blocks_activated){
        log_error("Blocks already activated");
        return false;
    }

    size_t total_size = 0;

    for(size_t i = 0; i < block_counter; i++){
        total_size += block_list[i].block_size;
    }

    void* temp = malloc(total_size);
    if(temp == NULL){
        log_error("Failed to allocate memory for main memory");
        return false;
    }
    main_memory = temp;

    uint8_t* current_addr = (uint8_t*)main_memory;
    for(size_t i = 0; i < block_counter; i++){
        block_list[i].addr = (void*)current_addr;
        current_addr += block_list[i].block_size;
    }

    blocks_activated = true;
    return true;
}

void* get_block_by_index(size_t block_index){
    if(block_index >= block_counter){
        return NULL;
    }
    return block_list[block_index].addr;
}
