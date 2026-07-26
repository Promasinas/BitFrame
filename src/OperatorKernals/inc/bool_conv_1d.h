/**
 * @file    bool_conv_1d.h
 * @brief   1-bit 1D convolution — declaration.
 *
 * @details All data is stored as packed bit arrays (8 bits per byte,
 *          LSB-first).  The convolution uses XOR + hardware popcount
 *          for acceleration: each output element is computed as
 *          @code
 *          output[o] = kernel_size − popcount(window ⊕ kernel)
 *          @endcode
 *          where `window` is a contiguous extract of `kernel_size`
 *          bits from the input at the current position, aligned to
 *          the kernel layout.
 *
 * @note    This is a **linear** (non-batched) 1D operator.  For
 *          upsampled convolution see bool_conv_1d_upsample.h.
 *
 * @par     Typical usage (dilation=1, no padding, stride=1):
 * @code
 *   size_t out_len = input_len - kernel_len + 1;
 *   uint32_t* out = malloc(out_len * sizeof(uint32_t));
 *   bool_conv_1d_forward(input, input_len,
 *                        kernel, kernel_len,
 *                        out, out_len,
 *                        1, 0, 1, false);
 * @endcode
 *
 * @see     bool_conv_1d_upsample_forward
 */

#ifndef __BOOL_CONV_1D_H__
#define __BOOL_CONV_1D_H__

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
 * @brief  1-bit 1D convolution forward pass.
 *
 * Slides a binary kernel over a binary input, computing at each
 * output position the number of bits that **match** (XNOR) between
 * the kernel and the corresponding input window.
 *
 * @param  input_addr   Packed input bits.  Must not be NULL.
 * @param  input_size   Total number of input bits (≥ 1).
 * @param  kernel_addr  Packed kernel bits.  Must not be NULL.
 * @param  kernel_size  Number of kernel bits (≥ 1).
 * @param  output_addr  Destination buffer for `uint32_t` results.
 *                      Caller must allocate at least
 *                      `output_size * sizeof(uint32_t)` bytes.
 * @param  output_size  Number of output positions to compute (≥ 1).
 *
 * @param  stride       Step between consecutive output positions
 *                      (≥ 1).  `stride = 1` computes every position;
 *                      `stride = 2` skips every other.
 *
 * @param  padding      Number of zero-bits prepended and appended
 *                      to the input.  `padding = 0` means "valid"
 *                      convolution; `padding > 0` pads both sides.
 *
 * @param  dilation     Spacing between kernel elements applied to
 *                      the input (≥ 1).  `dilation = 1` samples
 *                      adjacent bits; `dilation = 2` inserts one
 *                      gap between each kernel weight.
 *
 *                      The effective kernel span is
 *                      `(kernel_size − 1) * dilation + 1`.
 *
 * @param  is_loop      If `true`, treat the input as circular —
 *                      out-of-bounds accesses wrap modulo
 *                      `input_size`.  If `false`, out-of-bounds
 *                      positions yield 0 (zero-padding).
 *
 * @return `true` on success, `false` on invalid parameters or
 *         memory-allocation failure.
 *
 * @note   **Output-size formula** (caller is responsible for
 *         computing it correctly):
 * @code
 *   span  = (kernel_size - 1) * dilation + 1;
 *   o_cnt = (input_size + 2 * padding - span) / stride + 1;
 * @endcode
 *
 * @par    Performance
 *         - dilation=1 + linear + window-in-bounds →
 *           contiguous-memory extraction (memcpy / shifted copy).
 *         - Otherwise → per-bit gather into a packed window,
 *           then word-level (uint64_t) XOR + hardware POPCNT.
 *
 * @see    bool_conv_1d_upsample_forward
 */
BF_OPERATORKERNALS_API
bool bool_conv_1d_forward(void* input_addr,
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
