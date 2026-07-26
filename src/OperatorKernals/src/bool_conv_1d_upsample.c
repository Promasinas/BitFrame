/**
 * @file    bool_conv_1d_upsample.c
 * @brief   1-bit 1D convolution with input upsampling — implementation.
 *
 * @details Two code paths (mirrors bool_conv_1d_forward):
 *          1. **Fast path** — when `upsample_times == 1`, the
 *             operation is identical to bool_conv_1d_forward.
 *             Delegates directly to the same contiguous-extraction
 *             routine.  (dilation=1, linear, window-in-bounds.)
 *          2. **General path** — for `upsample_times > 1`, each
 *             kernel position maps to a *virtual* input position.
 *             The virtual input is the original input with
 *             `(upsample_times − 1)` zeros inserted between every
 *             pair of bits.  Positions that land on a zero gap
 *             contribute 0; positions aligned to an original
 *             sample extract the corresponding bit.
 *
 *          Virtual-position → original-position mapping:
 *            `v_pos % upsample_times == 0`
 *              → `bit = input[v_pos / upsample_times]`
 *            otherwise
 *              → `bit = 0`
 *
 *          Cyclic wrapping (`is_loop == true`) operates in the
 *          virtual space (size `v_size`), then maps back.
 */

#include "bool_conv_1d_upsample.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* ==================================================================
 * POPCOUNT64 — hardware popcount via compiler builtin
 * ================================================================== */
#ifdef _MSC_VER
#include <intrin.h>
#define POPCOUNT64(x) __popcnt64(x)
#else
#define POPCOUNT64(x) __builtin_popcountll(x)
#endif

/* ==================================================================
 * get_bit — extract a single bit from a packed array
 * ==================================================================
 * Layout: bit `pos` → byte `pos >> 3`, offset `pos & 7` (LSB-first).
 */
static inline bool get_bit(const uint8_t* packed, size_t pos)
{
    return (packed[pos >> 3] >> (pos & 7)) & 1;
}

/* ==================================================================
 * extract_contiguous — copy a bit-aligned window from packed input
 * ==================================================================
 * Same as in bool_conv_1d.c.  Used only by the fast path
 * (upsample_times == 1).  See that file for detailed comments.
 */
static void extract_contiguous(const uint8_t* input,
                               size_t          start_bit,
                               size_t          kernel_size,
                               uint8_t*        window)
{
    size_t kernel_bytes = (kernel_size + 7) / 8;
    size_t start_byte   = start_bit >> 3;
    size_t shift        = start_bit & 7;

    if (shift == 0) {
        memcpy(window, input + start_byte, kernel_bytes);
    } else {
        size_t inv_shift = 8 - shift;
        size_t end_byte  = start_byte + kernel_bytes;

        for (size_t src = start_byte, dst = 0; src < end_byte; src++, dst++) {
            window[dst] = (uint8_t)(
                (input[src]     >> shift) |
                (input[src + 1] << inv_shift)
            );
        }
    }
}

/* ==================================================================
 * bool_conv_1d_upsample_forward
 * ==================================================================
 * Full documentation in the header file.
 * ================================================================== */
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
                                   bool   is_loop)
{
    /* ---- parameter validation ---- */
    if (input_addr == NULL || kernel_addr == NULL || output_addr == NULL) {
        return false;
    }
    if (input_size == 0 || kernel_size == 0 || output_size == 0) {
        return false;
    }
    if (stride == 0 || dilation == 0 || upsample_times == 0) {
        return false;
    }

    const uint8_t* input  = (const uint8_t*)input_addr;
    const uint8_t* kernel = (const uint8_t*)kernel_addr;
    uint32_t*      output = (uint32_t*)output_addr;

    size_t kernel_bytes = (kernel_size + 7) / 8;

    /*
     * Virtual input size after zero-insertion.
     * Example: input_size=3, upsample_times=2 → v_size=5
     *   original: [a, b, c]
     *   virtual:  [a, 0, b, 0, c]
     */
    size_t virtual_size = (input_size - 1) * upsample_times + 1;

    /* ---- pre-compute XOR-loop boundaries ---- */
    const uint64_t* kernel64   = (const uint64_t*)kernel;
    size_t          full_words = kernel_size / 64;
    size_t          rem_bits   = kernel_size % 64;
    uint64_t        rem_mask   = (rem_bits > 0)
                                 ? ((1ULL << rem_bits) - 1)
                                 : 0;

    /* ---- allocate window (once, reused) ---- */
    uint8_t* window = (uint8_t*)malloc(kernel_bytes);
    if (window == NULL) {
        return false;
    }

    /* ==============================================================
     * Main convolution loop
     * ============================================================== */
    for (size_t o = 0; o < output_size; o++) {

        /* Start position in the virtual (upsampled) input. */
        int64_t v_start = (int64_t)(o * stride) - (int64_t)padding;

        /* ----------------------------------------------------------
         * FAST PATH
         *   upsample_times == 1  →  no zero-insertion
         *   dilation == 1, linear, window fully within [0, input_size)
         *
         * Identical to the fast path in bool_conv_1d_forward.
         * ---------------------------------------------------------- */
        if (upsample_times == 1 && dilation == 1 && !is_loop &&
            v_start >= 0 && (size_t)(v_start + kernel_size) <= input_size) {

            extract_contiguous(input, (size_t)v_start, kernel_size, window);

        } else {
            /* ------------------------------------------------------
             * GENERAL PATH
             *
             * For each kernel element `i`, compute the virtual
             * position `v_pos = v_start + i * dilation`.
             *
             * If `v_pos` falls on an original sample
             *   (v_pos % upsample_times == 0):
             *     → read input[v_pos / upsample_times]
             * Otherwise:
             *     → bit = 0  (this is an upsampled zero-gap)
             *
             * For cyclic mode, `v_pos` wraps modulo `virtual_size`
             * before the mapping.
             *
             * For linear mode, out-of-bounds `v_pos` yields 0
             * (the zero-padding convention).
             * ------------------------------------------------------ */
            memset(window, 0, kernel_bytes);

            for (size_t i = 0; i < kernel_size; i++) {
                int64_t v_pos = v_start + (int64_t)(i * dilation);
                bool    bit   = false;

                if (is_loop) {
                    /* ---- wrap in virtual (upsampled) space ---- */
                    int64_t wrapped = v_pos % (int64_t)virtual_size;
                    if (wrapped < 0) {
                        wrapped += (int64_t)virtual_size;
                    }
                    if ((size_t)wrapped % upsample_times == 0) {
                        size_t orig = (size_t)wrapped / upsample_times;
                        bit = get_bit(input, orig);
                    }
                    /* else: wrapped position falls on an upsampled zero */
                } else {
                    /* ---- linear: bounds-check, then map ---- */
                    if (v_pos >= 0 && (size_t)v_pos < virtual_size) {
                        if ((size_t)v_pos % upsample_times == 0) {
                            size_t orig = (size_t)v_pos / upsample_times;
                            bit = get_bit(input, orig);
                        }
                    }
                    /* else: v_pos is in the padding region → 0 */
                }

                if (bit) {
                    size_t byte_idx = i >> 3;
                    size_t bit_pos  = i & 7;
                    window[byte_idx] |= (uint8_t)(1u << bit_pos);
                }
            }
        }

        /* ----------------------------------------------------------
         * XOR + popcount core
         *
         *   output[o] = kernel_size − Σ popcount(kernel_w ^ window_w)
         *
         * Identical to bool_conv_1d_forward.
         * ---------------------------------------------------------- */
        uint64_t* window64   = (uint64_t*)window;
        size_t    mismatches = 0;

        for (size_t w = 0; w < full_words; w++) {
            mismatches += POPCOUNT64(kernel64[w] ^ window64[w]);
        }

        if (rem_bits > 0) {
            uint64_t xor_partial =
                (kernel64[full_words] ^ window64[full_words]) & rem_mask;
            mismatches += POPCOUNT64(xor_partial);
        }

        output[o] = (uint32_t)(kernel_size - mismatches);
    }

    free(window);
    return true;
}
