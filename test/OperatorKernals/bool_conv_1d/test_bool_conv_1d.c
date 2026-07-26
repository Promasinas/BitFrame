#include "bool_conv_1d.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// ============================================================
// Helpers for building packed bit arrays from string patterns
// ============================================================

// "0101 0011" → packed uint8_t array (spaces ignored)
static uint8_t* bits_from_string(const char* str, size_t* out_len) {
    size_t len = strlen(str);
    size_t bit_count = 0;
    for (size_t i = 0; i < len; i++) {
        if (str[i] == '0' || str[i] == '1') bit_count++;
    }
    size_t bytes = (bit_count + 7) / 8;
    uint8_t* packed = (uint8_t*)calloc(bytes, 1);
    size_t bit_idx = 0;
    for (size_t i = 0; i < len; i++) {
        if (str[i] == '1') {
            packed[bit_idx >> 3] |= (uint8_t)(1u << (bit_idx & 7));
            bit_idx++;
        } else if (str[i] == '0') {
            bit_idx++;
        }
    }
    *out_len = bit_count;
    return packed;
}

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) do { \
    tests_run++; \
    printf("  %-50s ", name); \
} while(0)

#define PASS() do { \
    tests_passed++; \
    printf("PASS\n"); \
} while(0)

#define ASSERT_EQ(a, b, msg) do { \
    if ((a) != (b)) { \
        printf("FAIL: %s (expected %zu, got %zu)\n", msg, (size_t)(b), (size_t)(a)); \
        return; \
    } \
} while(0)

#define ASSERT_TRUE(cond, msg) do { \
    if (!(cond)) { \
        printf("FAIL: %s\n", msg); \
        return; \
    } \
} while(0)

// ============================================================
// Test 1: Basic dilation=1, stride=1, no padding
//   input:  1 0 1 1 0 1 0 0
//   kernel: 1 0 1
//   pos0: I[0,1,2] = 1,0,1 vs K=1,0,1 → XNOR=1+1+1=3
//   pos1: I[1,2,3] = 0,1,1 vs K=1,0,1 → XNOR=0+0+1=1
//   pos2: I[2,3,4] = 1,1,0 vs K=1,0,1 → XNOR=1+0+0=1
//   pos3: I[3,4,5] = 1,0,1 vs K=1,0,1 → XNOR=1+1+1=3
//   pos4: I[4,5,6] = 0,1,0 vs K=1,0,1 → XNOR=0+0+0=0
//   pos5: I[5,6,7] = 1,0,0 vs K=1,0,1 → XNOR=1+1+0=2
// ============================================================
static void test_basic_conv(void) {
    TEST("basic dilation=1 stride=1 conv");

    const char* input_str  = "10110100";
    const char* kernel_str = "101";
    size_t input_len, kernel_len;
    uint8_t* input  = bits_from_string(input_str,  &input_len);
    uint8_t* kernel = bits_from_string(kernel_str, &kernel_len);

    size_t output_len = 6;  // 8 - 3 + 1
    uint32_t* output = (uint32_t*)calloc(output_len, sizeof(uint32_t));

    bool ok = bool_conv_1d_forward(input, input_len,
                                   kernel, kernel_len,
                                   output, output_len,
                                   1, 0, 1, false);
    ASSERT_TRUE(ok, "function returned false");

    uint32_t expected[] = {3, 1, 1, 3, 0, 2};
    for (size_t i = 0; i < output_len; i++) {
        ASSERT_EQ(output[i], expected[i], "wrong output value");
    }

    free(input); free(kernel); free(output);
    PASS();
}

// ============================================================
// Test 2: With padding
//   input:  1 1 0 1
//   kernel: 1 1 1
//   padding: 1  → virtual input: [0] 1 1 0 1 [0]
//   pos0: start=-1 → I[-1]=0,I[0]=1,I[1]=1 vs 1,1,1 → 0+1+1=2
//   pos1: start=0  → I[0]=1,I[1]=1,I[2]=0  vs 1,1,1 → 1+1+0=2
//   pos2: start=1  → I[1]=1,I[2]=0,I[3]=1  vs 1,1,1 → 1+0+1=2
//   pos3: start=2  → I[2]=0,I[3]=1,I[4]=0  vs 1,1,1 → 0+1+0=1
// ============================================================
static void test_padding(void) {
    TEST("conv with padding=1");

    const char* input_str  = "1101";
    const char* kernel_str = "111";
    size_t input_len, kernel_len;
    uint8_t* input  = bits_from_string(input_str,  &input_len);
    uint8_t* kernel = bits_from_string(kernel_str, &kernel_len);

    size_t output_len = 4;  // 4 + 2*1 - 3 + 1
    uint32_t* output = (uint32_t*)calloc(output_len, sizeof(uint32_t));

    bool ok = bool_conv_1d_forward(input, input_len,
                                   kernel, kernel_len,
                                   output, output_len,
                                   1, 1, 1, false);
    ASSERT_TRUE(ok, "function returned false");

    uint32_t expected[] = {2, 2, 2, 1};
    for (size_t i = 0; i < output_len; i++) {
        ASSERT_EQ(output[i], expected[i], "wrong output value");
    }

    free(input); free(kernel); free(output);
    PASS();
}

// ============================================================
// Test 3: Stride=2
//   input:  1 0 1 1 0 1 0 0
//   kernel: 1 0
//   stride: 2, no padding
//   pos0: start=0 → I[0]=1,I[1]=0 vs 1,0 → 1+1=2
//   pos1: start=2 → I[2]=1,I[3]=1 vs 1,0 → 1+0=1
//   pos2: start=4 → I[4]=0,I[5]=1 vs 1,0 → 0+0=0
//   pos3: start=6 → I[6]=0,I[7]=0 vs 1,0 → 0+1=1
// ============================================================
static void test_stride(void) {
    TEST("conv with stride=2");

    const char* input_str  = "10110100";
    const char* kernel_str = "10";
    size_t input_len, kernel_len;
    uint8_t* input  = bits_from_string(input_str,  &input_len);
    uint8_t* kernel = bits_from_string(kernel_str, &kernel_len);

    size_t output_len = 4;  // (8 - 2) / 2 + 1
    uint32_t* output = (uint32_t*)calloc(output_len, sizeof(uint32_t));

    bool ok = bool_conv_1d_forward(input, input_len,
                                   kernel, kernel_len,
                                   output, output_len,
                                   2, 0, 1, false);
    ASSERT_TRUE(ok, "function returned false");

    uint32_t expected[] = {2, 1, 0, 1};
    for (size_t i = 0; i < output_len; i++) {
        ASSERT_EQ(output[i], expected[i], "wrong output value");
    }

    free(input); free(kernel); free(output);
    PASS();
}

// ============================================================
// Test 4: Dilation=2
//   input:  1 0 1 1 0 1 0 0 1 0
//   kernel: 1 0 1
//   dilation=2: samples positions start+0, start+2, start+4
//   pos0: start=0 → I[0]=1,I[2]=1,I[4]=0 vs 1,0,1 → XNOR=1+0+0=1
//   pos1: start=1 → I[1]=0,I[3]=1,I[5]=1 vs 1,0,1 → XNOR=0+0+1=1
//   pos2: start=2 → I[2]=1,I[4]=0,I[6]=0 vs 1,0,1 → XNOR=1+1+0=2
//   pos3: start=3 → I[3]=1,I[5]=1,I[7]=0 vs 1,0,1 → XNOR=1+0+0=1
//   pos4: start=4 → I[4]=0,I[6]=0,I[8]=1 vs 1,0,1 → XNOR=0+1+1=2
//   pos5: start=5 → I[5]=1,I[7]=0,I[9]=0 vs 1,0,1 → XNOR=1+1+0=2
// ============================================================
static void test_dilation(void) {
    TEST("conv with dilation=2");

    const char* input_str  = "1011010010";
    const char* kernel_str = "101";
    size_t input_len, kernel_len;
    uint8_t* input  = bits_from_string(input_str,  &input_len);
    uint8_t* kernel = bits_from_string(kernel_str, &kernel_len);

    // Effective span = (3-1)*2 + 1 = 5, output = 10 - 5 + 1 = 6
    size_t output_len = 6;
    uint32_t* output = (uint32_t*)calloc(output_len, sizeof(uint32_t));

    bool ok = bool_conv_1d_forward(input, input_len,
                                   kernel, kernel_len,
                                   output, output_len,
                                   1, 0, 2, false);
    ASSERT_TRUE(ok, "function returned false");

    uint32_t expected[] = {1, 1, 2, 1, 2, 2};
    for (size_t i = 0; i < output_len; i++) {
        ASSERT_EQ(output[i], expected[i], "wrong output value");
    }

    free(input); free(kernel); free(output);
    PASS();
}

// ============================================================
// Test 5: Cyclic (is_loop=true) convolution
//   input:  1 0 0 1 1
//   kernel: 1 0 1
//   stride=1, no padding
//   pos0: I[0]=1,I[1]=0,I[2]=0 vs 1,0,1 → XNOR=1+1+0=2
//   pos1: I[1]=0,I[2]=0,I[3]=1 vs 1,0,1 → XNOR=0+1+1=2
//   pos2: I[2]=0,I[3]=1,I[4]=1 vs 1,0,1 → XNOR=0+0+1=1
//   pos3: I[3]=1,I[4]=1,I[0]=1 vs 1,0,1 → XNOR=1+0+1=2
//   pos4: I[4]=1,I[0]=1,I[1]=0 vs 1,0,1 → XNOR=1+0+0=1
// ============================================================
static void test_cyclic(void) {
    TEST("cyclic (is_loop=true) conv");

    const char* input_str  = "10011";
    const char* kernel_str = "101";
    size_t input_len, kernel_len;
    uint8_t* input  = bits_from_string(input_str,  &input_len);
    uint8_t* kernel = bits_from_string(kernel_str, &kernel_len);

    size_t output_len = 5;  // cyclic: same as input for stride=1
    uint32_t* output = (uint32_t*)calloc(output_len, sizeof(uint32_t));

    bool ok = bool_conv_1d_forward(input, input_len,
                                   kernel, kernel_len,
                                   output, output_len,
                                   1, 0, 1, true);
    ASSERT_TRUE(ok, "function returned false");

    uint32_t expected[] = {2, 2, 1, 2, 1};
    for (size_t i = 0; i < output_len; i++) {
        ASSERT_EQ(output[i], expected[i], "wrong output value");
    }

    free(input); free(kernel); free(output);
    PASS();
}

// ============================================================
// Test 6: NULL / zero-size parameter validation
// ============================================================
static void test_validation(void) {
    TEST("parameter validation");

    uint8_t input_buf[1] = {0};
    uint8_t kernel_buf[1] = {0};
    uint32_t output_buf[1] = {0};

    ASSERT_TRUE(!bool_conv_1d_forward(NULL, 8, kernel_buf, 3, output_buf, 6, 1, 0, 1, false),
                "should reject NULL input");
    ASSERT_TRUE(!bool_conv_1d_forward(input_buf, 8, NULL, 3, output_buf, 6, 1, 0, 1, false),
                "should reject NULL kernel");
    ASSERT_TRUE(!bool_conv_1d_forward(input_buf, 8, kernel_buf, 3, NULL, 6, 1, 0, 1, false),
                "should reject NULL output");
    ASSERT_TRUE(!bool_conv_1d_forward(input_buf, 0, kernel_buf, 3, output_buf, 6, 1, 0, 1, false),
                "should reject zero input_size");
    ASSERT_TRUE(!bool_conv_1d_forward(input_buf, 8, kernel_buf, 0, output_buf, 6, 1, 0, 1, false),
                "should reject zero kernel_size");
    ASSERT_TRUE(!bool_conv_1d_forward(input_buf, 8, kernel_buf, 3, output_buf, 0, 1, 0, 1, false),
                "should reject zero output_size");
    ASSERT_TRUE(!bool_conv_1d_forward(input_buf, 8, kernel_buf, 3, output_buf, 6, 0, 0, 1, false),
                "should reject zero stride");
    ASSERT_TRUE(!bool_conv_1d_forward(input_buf, 8, kernel_buf, 3, output_buf, 6, 1, 0, 0, false),
                "should reject zero dilation");

    PASS();
}

// ============================================================
// Test 7: Large kernel (tests word-level XOR path with mask)
//   Uses a 129-bit kernel to exercise the partial-word masking
// ============================================================
static void test_large_kernel(void) {
    TEST("large kernel (129 bits, mask path)");

    size_t input_len = 256;
    size_t input_bytes = (input_len + 7) / 8;
    uint8_t* input = (uint8_t*)calloc(input_bytes, 1);
    for (size_t i = 0; i < input_len; i += 2) {
        input[i >> 3] |= (uint8_t)(1u << (i & 7));
    }

    size_t kernel_len = 129;
    size_t kernel_bytes = (kernel_len + 7) / 8;
    uint8_t* kernel = (uint8_t*)calloc(kernel_bytes, 1);
    for (size_t i = 0; i < kernel_len; i++) {
        kernel[i >> 3] |= (uint8_t)(1u << (i & 7));
    }

    size_t output_len = input_len - kernel_len + 1;
    uint32_t* output = (uint32_t*)calloc(output_len, sizeof(uint32_t));

    bool ok = bool_conv_1d_forward(input, input_len,
                                   kernel, kernel_len,
                                   output, output_len,
                                   1, 0, 1, false);
    ASSERT_TRUE(ok, "function returned false");

    // pos0: input[0..128] vs all-ones kernel
    //   Input has 1s at positions 0,2,4,...,128 → 65 matches
    ASSERT_EQ(output[0], 65, "pos 0");

    // pos1: input[1..129] vs all-ones kernel
    //   Input has 1s at even positions: 2,4,...,128 → 64 matches
    ASSERT_EQ(output[1], 64, "pos 1");

    free(input); free(kernel); free(output);
    PASS();
}

// ============================================================
// Test 8: Dilation + stride + padding combined
//   input:  1 0 1 1 0 1 0
//   kernel: 1 0
//   dilation=2, stride=2, padding=1
//   Virtual input: [0] 1 0 1 1 0 1 0 [0]
//   Effective span: (2-1)*2 + 1 = 3
//   Output count: (7 + 2*1 - 3) / 2 + 1 = 3
//   pos0: start=0*2-1=-1 → I[-1]=0,I[1]=0 vs K=1,0 → XNOR=0+1=1
//   pos1: start=1*2-1=1  → I[1]=0,I[3]=1  vs K=1,0 → XNOR=0+0=0
//   pos2: start=2*2-1=3  → I[3]=1,I[5]=1  vs K=1,0 → XNOR=1+0=1
// ============================================================
static void test_combined(void) {
    TEST("dilation=2 stride=2 padding=1 combined");

    const char* input_str  = "1011010";
    const char* kernel_str = "10";
    size_t input_len, kernel_len;
    uint8_t* input  = bits_from_string(input_str,  &input_len);
    uint8_t* kernel = bits_from_string(kernel_str, &kernel_len);

    size_t output_len = 3;
    uint32_t* output = (uint32_t*)calloc(output_len, sizeof(uint32_t));

    bool ok = bool_conv_1d_forward(input, input_len,
                                   kernel, kernel_len,
                                   output, output_len,
                                   2, 1, 2, false);
    ASSERT_TRUE(ok, "function returned false");

    uint32_t expected[] = {1, 0, 1};
    for (size_t i = 0; i < output_len; i++) {
        ASSERT_EQ(output[i], expected[i], "wrong output value");
    }

    free(input); free(kernel); free(output);
    PASS();
}

// ============================================================
// Test 9: kernel_size=1 (identity-like)
// ============================================================
static void test_kernel_size_one(void) {
    TEST("kernel_size=1 (identity-like)");

    const char* input_str  = "10110";
    const char* kernel_str = "1";
    size_t input_len, kernel_len;
    uint8_t* input  = bits_from_string(input_str,  &input_len);
    uint8_t* kernel = bits_from_string(kernel_str, &kernel_len);

    size_t output_len = 5;
    uint32_t* output = (uint32_t*)calloc(output_len, sizeof(uint32_t));

    bool ok = bool_conv_1d_forward(input, input_len,
                                   kernel, kernel_len,
                                   output, output_len,
                                   1, 0, 1, false);
    ASSERT_TRUE(ok, "function returned false");

    // Kernel is [1], so output[i] = XNOR(input[i], 1) = input[i]
    uint32_t expected[] = {1, 0, 1, 1, 0};
    for (size_t i = 0; i < output_len; i++) {
        ASSERT_EQ(output[i], expected[i], "wrong output value");
    }

    free(input); free(kernel); free(output);
    PASS();
}

// ============================================================
int main(void) {
    printf("\n=== bool_conv_1d tests ===\n\n");

    test_basic_conv();
    test_padding();
    test_stride();
    test_dilation();
    test_cyclic();
    test_validation();
    test_large_kernel();
    test_combined();
    test_kernel_size_one();

    printf("\n=== Results: %d/%d passed ===\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
