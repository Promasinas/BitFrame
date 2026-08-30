#ifndef __BOOL_CONV_1D_H__
#define __BOOL_CONV_1D_H__

#include <stdbool.h>
#include <stddef.h>
#include "MemoryManagement.h"

bool bool_conv_1d_forward(size_t input_block_index,
                          size_t kernel_block_index,
                          size_t output_block_index,
                          size_t stride,
                          size_t padding,
                          size_t dilation,
                          bool   is_loop);

bool bool_conv_1d_forward_by_addr(void* input_addr,
                                  size_t input_size,
                                  void* kernel_addr,
                                  size_t kernel_size,
                                  void* output_addr,
                                  size_t output_size,
                                  size_t stride,
                                  size_t padding,
                                  size_t dilation,
                                  bool   is_loop);

#endif /* __BOOL_CONV_1D_H__ */
