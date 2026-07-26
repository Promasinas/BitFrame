/**
 * @file    bool_conv_1d_upsample.h
 * @brief   1-bit 1D convolution with input upsampling — declaration.
 *
 * @details Before applying the kernel, the input is **upsampled** by
 *          inserting `(upsample_times − 1)` zeros between every pair
 *          of adjacent input bits.  This creates a *virtual* input of
 *          length
 *          @code
 *          v_size = (input_size − 1) * upsample_times + 1
 *          @endcode
 *          against which standard 1D convolution is then run.
 *
 *          The virtual-position → original-position mapping is:
 *          - `v_pos % upsample_times == 0` → `input[v_pos / upsample_times]`
 *          - otherwise → `0` (upsampled zero)
 *
 * @note    When `upsample_times == 1` the behaviour is identical to
 *          bool_conv_1d_forward() and the same fast path (contiguous
 *          extraction) is taken.
 *
 * @par     Example (upsample_times = 2):
 * @code
 *   original:     [a, b, c]
 *   virtual:      [a, 0, b, 0, c]          (len = 5)
 *   conv (K=[k0,k1], dil=1, pad=0, str=1):
 *     o[0] = XNOR(k0,a) + XNOR(k1,0)
 *     o[1] = XNOR(k0,0) + XNOR(k1,b)
 *     o[2] = XNOR(k0,b) + XNOR(k1,0)
 *     o[3] = XNOR(k0,0) + XNOR(k1,c)
 * @endcode
 *
 * @see     bool_conv_1d_forward
 */

#ifndef __BOOL_CONV_1D_UPSAMPLE_H__
#define __BOOL_CONV_1D_UPSAMPLE_H__

#include <stdbool.h>
#include <stddef.h>

/* ------------------------------------------------------------------
 * DLL export / import
 * ------------------------------------------------------------------ */
#ifndef BF_OPERATORKERNALS_API
  #if defined(_WIN32)
    #ifdef BF_OPERATORKERNALS_EXPORTS
      #define BF_OPERATORKERNALS_API __declspec(dllexport)
    #else
      #define BF_OPERATORKERNALS_API __declspec(dllimport)
    #endif
  #else
    #define BF_OPERATORKERNALS_API \
      __attribute__((visibility("default")))
  #endif
#endif

/**
 * @brief  1-bit 1D convolution with input upsampling, forward pass.
 *
 * Inserts `(upsample_times − 1)` zeros between each input bit, then
 * applies standard 1D convolution on the resulting virtual input.
 *
 * @param  input_addr     Packed input bits.  Must not be NULL.
 * @param  input_size     Number of original input bits (≥ 1).
 * @param  kernel_addr    Packed kernel bits.  Must not be NULL.
 * @param  kernel_size    Number of kernel bits (≥ 1).
 * @param  output_addr    Destination buffer for `uint32_t` results.
 * @param  output_size    Number of output positions (≥ 1).
 *
 * @param  upsample_times Upsampling factor (≥ 1).  `1` = no change;
 *                        `2` = insert one zero between each pair of
 *                        input bits; `3` = two zeros, etc.
 *
 * @param  stride         Step between output positions (≥ 1).
 * @param  padding        Zero-padding on both sides of the **virtual**
 *                        input (≥ 0).
 * @param  dilation       Kernel spacing on the virtual input (≥ 1).
 * @param  is_loop        `true` → cyclic wrap in virtual space;
 *                        `false` → linear with zero-padding.
 *
 * @return `true` on success, `false` on invalid parameters or
 *         memory-allocation failure.
 *
 * @note   **Output-size formula:**
 * @code
 *   v_size = (input_size - 1) * upsample_times + 1;
 *   span   = (kernel_size - 1) * dilation + 1;
 *   o_cnt  = (v_size + 2 * padding - span) / stride + 1;
 * @endcode
 *
 * @par    Performance
 *         - `upsample_times == 1` → delegates to the same fast path
 *           as bool_conv_1d_forward (contiguous extraction).
 *         - `upsample_times > 1`  → per-bit gather from virtual
 *           space, then word-level XOR + hardware POPCNT.
 *
 * @see    bool_conv_1d_forward
 */
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
                                   bool   is_loop);

#endif /* __BOOL_CONV_1D_UPSAMPLE_H__ */
