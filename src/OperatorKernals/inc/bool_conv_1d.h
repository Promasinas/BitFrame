#ifndef __BOOL_CONV_1D_H__
#define __BOOL_CONV_1D_H__

#include <stdbool.h>
#include <stddef.h>

bool bool_conv_1d_forward(void* input_addr,
                          size_t input_size,
                          void* kernel_addr,
                          size_t kernel_size,
                          void* output_addr,
                          size_t output_size,
                          size_t stride,
                          size_t padding,
                          size_t dilation,
                          bool is_loop);

#endif