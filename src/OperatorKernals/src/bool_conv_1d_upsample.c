#include "bool_conv_1d_upsample.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

// ============================================================
// Popcount — hardware POPCNT via compiler builtin
// ============================================================
#ifdef _MSC_VER
#include <intrin.h>
#define POPCOUNT64(x) __popcnt64(x)
#else
#define POPCOUNT64(x) __builtin_popcountll(x)
#endif

// ============================================================
// Bit-level access for packed bit arrays
// Layout: bit N → byte[N/8], LSB-first within byte
// ============================================================
static inline bool get_bit(const uint8_t* packed, size_t pos) {
    return (packed[pos >> 3] >> (pos & 7)) & 1;
}

// ============================================================
// Fast extraction: copy kernel_size contiguous bits from
// input[start_bit] into window, handling bit-level alignment.
// ============================================================
static void extract_contiguous(const uint8_t* input,
                               size_t start_bit,
                               size_t kernel_size,
                               uint8_t* window)
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

// ============================================================
// bool_conv_1d_upsample_forward
// ============================================================
// Inserts (upsample_times − 1) zeros between each input bit,
// then runs standard 1D convolution on the virtual upsampled
// input.  The mapping from virtual position to original input:
//
//   if virtual_pos % upsample_times == 0:
//       bit = input[virtual_pos / upsample_times]
//   else:
//       bit = 0   (upsampled zero insertion)
//
// Fast path: when upsample_times == 1 this is identical to
// bool_conv_1d_forward and uses the same contiguous-extraction
// optimisation.
// ============================================================
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
                                   bool is_loop)
{
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

    // Virtual input size after zero-insertion
    size_t virtual_size = (input_size - 1) * upsample_times + 1;

    const uint64_t* kernel64   = (const uint64_t*)kernel;
    size_t          full_words = kernel_size / 64;
    size_t          rem_bits   = kernel_size % 64;
    uint64_t        rem_mask   = (rem_bits > 0)
                                 ? ((1ULL << rem_bits) - 1)
                                 : 0;

    uint8_t* window = (uint8_t*)malloc(kernel_bytes);
    if (window == NULL) {
        return false;
    }

    for (size_t o = 0; o < output_size; o++) {

        int64_t v_start = (int64_t)(o * stride) - (int64_t)padding;

        // FAST PATH: upsample_times == 1  →  identical to regular conv
        if (upsample_times == 1 && dilation == 1 && !is_loop &&
            v_start >= 0 && (size_t)(v_start + kernel_size) <= input_size) {

            extract_contiguous(input, (size_t)v_start, kernel_size, window);

        } else {
            // GENERAL PATH: upsample mapping  v_pos → orig bit
            memset(window, 0, kernel_bytes);

            for (size_t i = 0; i < kernel_size; i++) {
                int64_t v_pos = v_start + (int64_t)(i * dilation);
                bool    bit   = false;

                if (is_loop) {
                    int64_t wrapped = v_pos % (int64_t)virtual_size;
                    if (wrapped < 0) {
                        wrapped += (int64_t)virtual_size;
                    }
                    if ((size_t)wrapped % upsample_times == 0) {
                        size_t orig = (size_t)wrapped / upsample_times;
                        bit = get_bit(input, orig);
                    }
                } else {
                    if (v_pos >= 0 && (size_t)v_pos < virtual_size) {
                        if ((size_t)v_pos % upsample_times == 0) {
                            size_t orig = (size_t)v_pos / upsample_times;
                            bit = get_bit(input, orig);
                        }
                    }
                }

                if (bit) {
                    size_t byte_idx = i >> 3;
                    size_t bit_pos  = i & 7;
                    window[byte_idx] |= (uint8_t)(1u << bit_pos);
                }
            }
        }

        // XOR + popcount core
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
