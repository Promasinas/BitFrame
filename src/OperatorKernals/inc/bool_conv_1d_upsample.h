#ifndef __BOOL_CONV_1D_UPSAMPLE_H__
#define __BOOL_CONV_1D_UPSAMPLE_H__

#include <stdbool.h>
#include <stddef.h>

// --- DLL export / import ----------------------------------------
#ifndef BF_OPERATORKERNALS_API
  #if defined(_WIN32)
    #ifdef BF_OPERATORKERNALS_EXPORTS
      #define BF_OPERATORKERNALS_API __declspec(dllexport)
    #else
      #define BF_OPERATORKERNALS_API __declspec(dllimport)
    #endif
  #else
    #define BF_OPERATORKERNALS_API __attribute__((visibility("default")))
  #endif
#endif

BF_OPERATORKERNALS_API
bool bool_conv_1d_upsample_forward(void* input_addr,
                                   size_t input_size,
                                   void* kernel_addr,
                                   size_t kernel_size,
                                   void* output_addr,
                                   size_t output_size,
                                   size_t upsample_times,
                                   size_t stride,
                                   size_t padding,
                                   size_t dilation,
                                   bool is_loop);

#endif
