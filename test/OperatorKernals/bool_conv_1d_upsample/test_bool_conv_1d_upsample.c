#include "bool_conv_1d.h"
#include "bool_conv_1d_upsample.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// ============================================================
// Helpers
// ============================================================
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

#define TEST(name) do { tests_run++; printf("  %-50s ", name); } while(0)
#define PASS()     do { tests_passed++; printf("PASS\n"); } while(0)

#define ASSERT_EQ(a, b, msg) do { \
    if ((a) != (b)) { \
        printf("FAIL: %s (expected %zu, got %zu)\n", msg, (size_t)(b), (size_t)(a)); \
        return; \
    } \
} while(0)

#define ASSERT_TRUE(cond, msg) do { \
    if (!(cond)) { printf("FAIL: %s\n", msg); return; } \
} while(0)

// ============================================================
// Test 1: upsample_times=1 identical to regular conv
// ============================================================
static void test_upsample_times_one(void) {
    TEST("upsample times=1 (≡ regular conv)");

    const char* input_str  = "10110100";
    const char* kernel_str = "101";
    size_t input_len, kernel_len;
    uint8_t* input  = bits_from_string(input_str,  &input_len);
    uint8_t* kernel = bits_from_string(kernel_str, &kernel_len);

    size_t output_len = 6;
    uint32_t* out_reg  = (uint32_t*)calloc(output_len, sizeof(uint32_t));
    uint32_t* out_up   = (uint32_t*)calloc(output_len, sizeof(uint32_t));

    bool ok1 = bool_conv_1d_forward(input, input_len,
                                    kernel, kernel_len,
                                    out_reg, output_len,
                                    1, 0, 1, false);
    bool ok2 = bool_conv_1d_upsample_forward(input, input_len,
                                             kernel, kernel_len,
                                             out_up, output_len,
                                             1,
                                             1, 0, 1, false);
    ASSERT_TRUE(ok1 && ok2, "function returned false");

    for (size_t i = 0; i < output_len; i++) {
        ASSERT_EQ(out_up[i], out_reg[i], "upsample×1 must match regular conv");
    }

    free(input); free(kernel); free(out_reg); free(out_up);
    PASS();
}

// ============================================================
// Test 2: upsample_times=2, stride=1
//   input:  1 0 1
//   kernel: 1 1
//   upsample=2 → virtual: [1,0, 0,0, 1] (len=5)
//   pos0: v[0,1]=1,0 vs K=1,1 → XNOR=1+0=1
//   pos1: v[1,2]=0,0 vs K=1,1 → XNOR=0+0=0
//   pos2: v[2,3]=0,0 vs K=1,1 → XNOR=0+0=0
//   pos3: v[3,4]=0,1 vs K=1,1 → XNOR=0+1=1
// ============================================================
static void test_upsample_basic(void) {
    TEST("upsample times=2 basic");

    const char* input_str  = "101";
    const char* kernel_str = "11";
    size_t input_len, kernel_len;
    uint8_t* input  = bits_from_string(input_str,  &input_len);
    uint8_t* kernel = bits_from_string(kernel_str, &kernel_len);

    size_t output_len = 4;
    uint32_t* output = (uint32_t*)calloc(output_len, sizeof(uint32_t));

    bool ok = bool_conv_1d_upsample_forward(input, input_len,
                                            kernel, kernel_len,
                                            output, output_len,
                                            2,
                                            1, 0, 1, false);
    ASSERT_TRUE(ok, "function returned false");

    uint32_t expected[] = {1, 0, 0, 1};
    for (size_t i = 0; i < output_len; i++) {
        ASSERT_EQ(output[i], expected[i], "wrong output");
    }

    free(input); free(kernel); free(output);
    PASS();
}

// ============================================================
// Test 3: upsample_times=3
//   input:  1 0
//   kernel: 1
//   upsample=3 → virtual: [1,0,0, 0] (len=4)
//   pos0: v[0]=1 → 1
//   pos1: v[1]=0 → 0
//   pos2: v[2]=0 → 0
//   pos3: v[3]=0 → 0
// ============================================================
static void test_upsample_times_three(void) {
    TEST("upsample times=3");

    const char* input_str  = "10";
    const char* kernel_str = "1";
    size_t input_len, kernel_len;
    uint8_t* input  = bits_from_string(input_str,  &input_len);
    uint8_t* kernel = bits_from_string(kernel_str, &kernel_len);

    size_t output_len = 4;
    uint32_t* output = (uint32_t*)calloc(output_len, sizeof(uint32_t));

    bool ok = bool_conv_1d_upsample_forward(input, input_len,
                                            kernel, kernel_len,
                                            output, output_len,
                                            3,
                                            1, 0, 1, false);
    ASSERT_TRUE(ok, "function returned false");

    uint32_t expected[] = {1, 0, 0, 0};
    for (size_t i = 0; i < output_len; i++) {
        ASSERT_EQ(output[i], expected[i], "wrong output");
    }

    free(input); free(kernel); free(output);
    PASS();
}

// ============================================================
// Test 4: Upsample with stride=2
//   input:  1 0 1 1
//   kernel: 1 0
//   upsample=2 → virtual: [1,0, 0,0, 1,0, 1] (len=7)
//   stride=2
//   pos0: v[0,1]=1,0 vs K=1,0 → 1+1=2
//   pos1: v[2,3]=0,0 vs K=1,0 → 0+1=1
//   pos2: v[4,5]=1,0 vs K=1,0 → 1+1=2
// ============================================================
static void test_upsample_stride(void) {
    TEST("upsample times=2 stride=2");

    const char* input_str  = "1011";
    const char* kernel_str = "10";
    size_t input_len, kernel_len;
    uint8_t* input  = bits_from_string(input_str,  &input_len);
    uint8_t* kernel = bits_from_string(kernel_str, &kernel_len);

    size_t output_len = 3;
    uint32_t* output = (uint32_t*)calloc(output_len, sizeof(uint32_t));

    bool ok = bool_conv_1d_upsample_forward(input, input_len,
                                            kernel, kernel_len,
                                            output, output_len,
                                            2,
                                            2, 0, 1, false);
    ASSERT_TRUE(ok, "function returned false");

    uint32_t expected[] = {2, 1, 2};
    for (size_t i = 0; i < output_len; i++) {
        ASSERT_EQ(output[i], expected[i], "wrong output");
    }

    free(input); free(kernel); free(output);
    PASS();
}

// ============================================================
// Test 5: Upsample with dilation=2
//   input:  1 1 0
//   kernel: 1 0
//   upsample=2 → virtual: [1,0, 1,0, 0] (len=5)
//   dilation=2: kernel samples v_start+0, v_start+2
//   pos0: v[0]=1, v[2]=1 vs K=1,0 → 1+0=1
//   pos1: v[1]=0, v[3]=0 vs K=1,0 → 0+1=1
//   pos2: v[2]=1, v[4]=0 vs K=1,0 → 1+1=2
// ============================================================
static void test_upsample_dilation(void) {
    TEST("upsample times=2 dilation=2");

    const char* input_str  = "110";
    const char* kernel_str = "10";
    size_t input_len, kernel_len;
    uint8_t* input  = bits_from_string(input_str,  &input_len);
    uint8_t* kernel = bits_from_string(kernel_str, &kernel_len);

    size_t output_len = 3;
    uint32_t* output = (uint32_t*)calloc(output_len, sizeof(uint32_t));

    bool ok = bool_conv_1d_upsample_forward(input, input_len,
                                            kernel, kernel_len,
                                            output, output_len,
                                            2,
                                            1, 0, 2, false);
    ASSERT_TRUE(ok, "function returned false");

    uint32_t expected[] = {1, 1, 2};
    for (size_t i = 0; i < output_len; i++) {
        ASSERT_EQ(output[i], expected[i], "wrong output");
    }

    free(input); free(kernel); free(output);
    PASS();
}

// ============================================================
// Test 6: Upsample with padding=1
//   input:  1 0
//   kernel: 1 1
//   upsample=2 → virtual: [1,0, 0] (len=3)
//   padding=1 → padded virtual: [0, 1,0, 0, 0]
//   pos0: v[-1]=0, v[0]=1 → XNOR=0+1=1
//   pos1: v[0]=1,  v[1]=0 → XNOR=1+0=1
//   pos2: v[1]=0,  v[2]=0 → XNOR=0+0=0
//   pos3: v[2]=0,  v[3]=0 → XNOR=0+0=0
// ============================================================
static void test_upsample_padding(void) {
    TEST("upsample times=2 padding=1");

    const char* input_str  = "10";
    const char* kernel_str = "11";
    size_t input_len, kernel_len;
    uint8_t* input  = bits_from_string(input_str,  &input_len);
    uint8_t* kernel = bits_from_string(kernel_str, &kernel_len);

    size_t output_len = 4;
    uint32_t* output = (uint32_t*)calloc(output_len, sizeof(uint32_t));

    bool ok = bool_conv_1d_upsample_forward(input, input_len,
                                            kernel, kernel_len,
                                            output, output_len,
                                            2,
                                            1, 1, 1, false);
    ASSERT_TRUE(ok, "function returned false");

    uint32_t expected[] = {1, 1, 0, 0};
    for (size_t i = 0; i < output_len; i++) {
        ASSERT_EQ(output[i], expected[i], "wrong output");
    }

    free(input); free(kernel); free(output);
    PASS();
}

// ============================================================
// Test 7: Upsample with cyclic (is_loop=true)
//   input:  1 0 1
//   kernel: 1 1
//   upsample=2 → virtual: [1,0, 0,0, 1] (len=5)
//   pos0: v[0,1]=1,0 → 1+0=1
//   pos1: v[1,2]=0,0 → 0+0=0
//   pos2: v[2,3]=0,0 → 0+0=0
//   pos3: v[3,4]=0,1 → 0+1=1
//   pos4: v[4,0]=1,1 → 1+1=2  (wraps: v[5]→v[0]=1)
// ============================================================
static void test_upsample_cyclic(void) {
    TEST("upsample times=2 cyclic");

    const char* input_str  = "101";
    const char* kernel_str = "11";
    size_t input_len, kernel_len;
    uint8_t* input  = bits_from_string(input_str,  &input_len);
    uint8_t* kernel = bits_from_string(kernel_str, &kernel_len);

    size_t output_len = 5;
    uint32_t* output = (uint32_t*)calloc(output_len, sizeof(uint32_t));

    bool ok = bool_conv_1d_upsample_forward(input, input_len,
                                            kernel, kernel_len,
                                            output, output_len,
                                            2,
                                            1, 0, 1, true);
    ASSERT_TRUE(ok, "function returned false");

    uint32_t expected[] = {1, 0, 0, 1, 2};
    for (size_t i = 0; i < output_len; i++) {
        ASSERT_EQ(output[i], expected[i], "wrong output");
    }

    free(input); free(kernel); free(output);
    PASS();
}

// ============================================================
// Test 8: Parameter validation
// ============================================================
static void test_upsample_validation(void) {
    TEST("upsample parameter validation");

    uint8_t input_buf[1]  = {0};
    uint8_t kernel_buf[1] = {0};
    uint32_t output_buf[1] = {0};

    ASSERT_TRUE(!bool_conv_1d_upsample_forward(input_buf, 8,
                                               kernel_buf, 3,
                                               output_buf, 6,
                                               0,
                                               1, 0, 1, false),
                "should reject zero upsample_times");

    PASS();
}

// ============================================================
int main(void) {
    printf("\n=== bool_conv_1d_upsample tests ===\n\n");

    test_upsample_times_one();
    test_upsample_basic();
    test_upsample_times_three();
    test_upsample_stride();
    test_upsample_dilation();
    test_upsample_padding();
    test_upsample_cyclic();
    test_upsample_validation();

    printf("\n=== Results: %d/%d passed ===\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
