/**
 * @file    bool_conv_1d.c
 * @brief   1-bit 1D convolution — implementation.
 *
 * @details Optimised with two code paths:
 *          1. **Fast path** (dilation=1, linear, window fully in
 *             bounds) — extracts `kernel_size` contiguous bits
 *             directly from the packed input (memcpy or single-pass
 *             shift).  No per-bit gather, no memset.
 *          2. **General path** — gathers scattered bits (dilation>1,
 *             padding edges, or cyclic wrap) into a temporary packed
 *             window, then applies word-level XOR + hardware POPCNT.
 *
 *          Both paths share the same XOR+popcount core which operates
 *          on uint64_t words for maximum throughput.
 *
 * @note    All internal helpers (POPCOUNT64, get_bit, extract_contiguous)
 *          are file-local to avoid symbol leakage in shared-library builds.
 */

#include "bool_conv_1d.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* ==================================================================
 * POPCOUNT64 — hardware popcount via compiler builtin
 * ==================================================================
 * Maps to the x86 POPCNT instruction (or equivalent) for single-cycle
 * bit-counting on 64-bit words.  Fallback: __builtin_popcountll on
 * GCC/Clang; __popcnt64 on MSVC.
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
 * Packed layout: bit `pos` lives at byte `pos >> 3`, bit offset
 * `pos & 7` (LSB = bit 0 within each byte).
 *
 * @param packed  Pointer to the packed bit buffer.
 * @param pos     Logical bit index (0-based).
 * @return        0 or 1.
 * ================================================================== */
static inline bool get_bit(const uint8_t* packed, size_t pos)
{
    return (packed[pos >> 3] >> (pos & 7)) & 1;
}

/* ==================================================================
 * extract_contiguous — copy a bit-aligned window from packed input
 * ==================================================================
 * Copies `kernel_size` bits starting at `input[start_bit]` into
 * `window` as a dense packed array suitable for word-level XOR.
 *
 * **Precondition:** `start_bit >= 0` and
 * `start_bit + kernel_size <= input_size`.  Caller guarantees the
 * entire window lies within the input (no padding / wrap).
 *
 * **Performance:**
 *   - `start_bit % 8 == 0` → single `memcpy`.
 *   - otherwise            → one sequential pass of shift+combine.
 *
 * Each loop iteration reads two consecutive bytes from `input` and
 * writes one byte to `window`.  The access pattern is fully sequential
 * and cache-friendly.
 *
 * @param input       Pointer to the packed input buffer.
 * @param start_bit   Bit offset where the window begins.
 * @param kernel_size Number of bits to copy.
 * @param window      Destination packed buffer (caller-allocated,
 *                    at least `(kernel_size+7)/8` bytes).
 * ================================================================== */
static void extract_contiguous(const uint8_t* input,
                               size_t          start_bit,
                               size_t          kernel_size,
                               uint8_t*        window)
{
    size_t kernel_bytes = (kernel_size + 7) / 8;
    size_t start_byte   = start_bit >> 3;       // byte offset
    size_t shift        = start_bit & 7;        // bit offset within byte

    if (shift == 0) {
        /* ---- byte-aligned: pure contiguous copy ---- */
        memcpy(window, input + start_byte, kernel_bytes);
    } else {
        /* ---- misaligned: shift + combine each byte ---- */
        size_t inv_shift = 8 - shift;
        size_t end_byte  = start_byte + kernel_bytes;

        for (size_t src = start_byte, dst = 0; src < end_byte; src++, dst++) {
            window[dst] = (uint8_t)(
                (input[src]     >> shift) |      // lower  part from current byte
                (input[src + 1] << inv_shift)    // upper  part from next byte
            );
        }
    }
}

/* ==================================================================
 * bool_conv_1d_forward
 * ==================================================================
 * Full documentation in the header file.
 * ================================================================== */
bool bool_conv_1d_forward(void* input_addr,
                          size_t input_size,
                          void* kernel_addr,
                          size_t kernel_size,
                          void* output_addr,
                          size_t output_size,
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
    if (stride == 0 || dilation == 0) {
        return false;
    }

    const uint8_t* input  = (const uint8_t*)input_addr;
    const uint8_t* kernel = (const uint8_t*)kernel_addr;
    uint32_t*      output = (uint32_t*)output_addr;

    /* ---- allocate window (once, reused across output positions) ---- */
    size_t kernel_bytes = (kernel_size + 7) / 8;

    /* ---- pre-compute XOR-loop boundaries ---- */
    const uint64_t* kernel64   = (const uint64_t*)kernel;
    size_t          full_words = kernel_size / 64;       // # of full 64-bit words
    size_t          rem_bits   = kernel_size % 64;       // trailing bits (< 64)
    uint64_t        rem_mask   = (rem_bits > 0)
                                 ? ((1ULL << rem_bits) - 1)     // e.g. 0x7F for 7 bits
                                 : 0;                           // no remainder

    uint8_t* window = (uint8_t*)malloc(kernel_bytes);
    if (window == NULL) {
        return false;
    }

    /* ==============================================================
     * Main convolution loop  —  one iteration per output position
     * ============================================================== */
    for (size_t o = 0; o < output_size; o++) {

        /*
         * Start position in the (conceptually padded) input.
         * Negative → left-padding region; ≥ input_size → right-padding.
         */
        int64_t start = (int64_t)(o * stride) - (int64_t)padding;

        /* ----------------------------------------------------------
         * FAST PATH
         *   dilation == 1
         *   linear convolution (no wrap)
         *   entire window [start, start+kernel_size) lies within
         *     [0, input_size)
         *
         * → contiguous extraction, no memset needed (every byte is
         *   overwritten).
         * ---------------------------------------------------------- */
        if (dilation == 1 && !is_loop &&
            start >= 0 && (size_t)(start + kernel_size) <= input_size) {

            extract_contiguous(input, (size_t)start, kernel_size, window);

        } else {
            /* ------------------------------------------------------
             * GENERAL PATH
             *   dilation > 1  → scattered reads
             *   cyclic        → modulo wrap
             *   edge padding  → out-of-bounds bits = 0
             *
             * Build the packed window bit-by-bit, then XOR+popcount.
             * ------------------------------------------------------ */
            memset(window, 0, kernel_bytes);

            for (size_t i = 0; i < kernel_size; i++) {
                int64_t input_pos = start + (int64_t)(i * dilation);
                bool    bit       = false;

                if (is_loop) {
                    /* ---- cyclic: wrap around modulo input_size ---- */
                    int64_t wrapped = input_pos % (int64_t)input_size;
                    if (wrapped < 0) {
                        wrapped += (int64_t)input_size;
                    }
                    bit = get_bit(input, (size_t)wrapped);
                } else {
                    /* ---- linear: out-of-bounds → 0 ---- */
                    if (input_pos >= 0 && (size_t)input_pos < input_size) {
                        bit = get_bit(input, (size_t)input_pos);
                    }
                }

                if (bit) {
                    size_t byte_idx = i >> 3;
                    size_t bit_pos  = i & 7;
                    window[byte_idx] |= (uint8_t)(1u << bit_pos);
                }
            }
        }

        /* ----------------------------------------------------------
         * XOR + popcount core  (shared by both paths)
         *
         *   mismatches = Σ popcount( kernel_word[w] ^ window_word[w] )
         *   output[o]  = kernel_size − mismatches
         *
         * The subtraction gives us XNOR (bitwise equality) count.
         * ---------------------------------------------------------- */
        uint64_t* window64   = (uint64_t*)window;
        size_t    mismatches = 0;

        for (size_t w = 0; w < full_words; w++) {
            mismatches += POPCOUNT64(kernel64[w] ^ window64[w]);
        }

        if (rem_bits > 0) {
            /* Mask off padding bits in the last partial word.
             * Both kernel and window may have garbage in the
             * unused high bits; the mask ensures only the
             * `rem_bits` meaningful bits contribute. */
            uint64_t xor_partial =
                (kernel64[full_words] ^ window64[full_words]) & rem_mask;
            mismatches += POPCOUNT64(xor_partial);
        }

        output[o] = (uint32_t)(kernel_size - mismatches);
    }

    free(window);
    return true;
}
