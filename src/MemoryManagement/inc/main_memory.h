#ifndef __MAIN_MEMORY_H__
#define __MAIN_MEMORY_H__

#include <stdbool.h>
#include <stddef.h>

// --- DLL export / import ----------------------------------------
#ifndef BF_MEMORYMANAGEMENT_API
  #if defined(_WIN32)
    #ifdef BF_MEMORYMANAGEMENT_EXPORTS
      #define BF_MEMORYMANAGEMENT_API __declspec(dllexport)
    #else
      #define BF_MEMORYMANAGEMENT_API __declspec(dllimport)
    #endif
  #else
    #define BF_MEMORYMANAGEMENT_API __attribute__((visibility("default")))
  #endif
#endif

#define BLOCK_LIST_UNIT_SIZE 128

typedef struct {
    size_t block_size;
    void*  addr;
} memory_block_t;

BF_MEMORYMANAGEMENT_API void  clear_blocks(void);
BF_MEMORYMANAGEMENT_API void  clear_main_memory(void);
BF_MEMORYMANAGEMENT_API bool  add_block(size_t block_size);
BF_MEMORYMANAGEMENT_API bool  activate_blocks(void);
BF_MEMORYMANAGEMENT_API void* get_block_by_index(size_t block_index);

#endif
