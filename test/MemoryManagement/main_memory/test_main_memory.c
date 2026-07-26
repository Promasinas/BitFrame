#include "main_memory.h"
#include <stdio.h>
#include <stdint.h>

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) do { tests_run++; printf("  %-45s ", name); } while(0)
#define PASS()     do { tests_passed++; printf("PASS\n"); } while(0)
#define FAIL(msg)  do { printf("FAIL: %s\n", msg); return; } while(0)
#define CHECK(cond, msg) do { if (!(cond)) { FAIL(msg); } } while(0)

// ----------------------------------------------------------
// Each test cleans up after itself so static state is fresh.
// ----------------------------------------------------------

static void test_add_single_block(void) {
    TEST("add single block");
    clear_blocks();

    CHECK(add_block(64), "add_block(64) should succeed");
    CHECK(get_block_by_index(0) == NULL,
          "addr should be NULL before activate");

    clear_blocks();
    PASS();
}

static void test_add_multiple_blocks(void) {
    TEST("add multiple blocks");
    clear_blocks();

    // BLOCK_LIST_UNIT_SIZE is 128 — add 200 blocks to force realloc
    for (size_t i = 0; i < 200; i++) {
        CHECK(add_block(i + 1), "add_block should succeed");
    }

    clear_blocks();
    PASS();
}

static void test_activate_and_get(void) {
    TEST("activate blocks and get pointers");
    clear_blocks();

    add_block(128);
    add_block(256);
    add_block(64);

    CHECK(activate_blocks(), "activate_blocks should succeed");

    void* b0 = get_block_by_index(0);
    void* b1 = get_block_by_index(1);
    void* b2 = get_block_by_index(2);

    CHECK(b0 != NULL, "block 0 ptr should not be NULL");
    CHECK(b1 != NULL, "block 1 ptr should not be NULL");
    CHECK(b2 != NULL, "block 2 ptr should not be NULL");

    // Blocks must be contiguous in memory and non-overlapping.
    // b1 comes right after b0, b2 after b1.
    uint8_t* addr0 = (uint8_t*)b0;
    uint8_t* addr1 = (uint8_t*)b1;
    uint8_t* addr2 = (uint8_t*)b2;

    CHECK(addr0 + 128 == addr1, "block 1 should follow block 0 contiguously");
    CHECK(addr1 + 256 == addr2, "block 2 should follow block 1 contiguously");

    clear_main_memory();
    clear_blocks();
    PASS();
}

static void test_double_activate_fails(void) {
    TEST("double activate should fail");
    clear_blocks();

    add_block(32);
    CHECK(activate_blocks(), "first activate should succeed");
    CHECK(!activate_blocks(), "second activate should fail");

    clear_main_memory();
    clear_blocks();
    PASS();
}

static void test_get_out_of_bounds(void) {
    TEST("get_block_by_index out-of-bounds");
    clear_blocks();

    add_block(16);
    add_block(16);
    activate_blocks();

    CHECK(get_block_by_index(2) == NULL, "index 2 should be out of bounds");
    CHECK(get_block_by_index(999) == NULL, "index 999 should be out of bounds");

    clear_main_memory();
    clear_blocks();
    PASS();
}

static void test_clear_blocks_resets_state(void) {
    TEST("clear_blocks resets state");
    clear_blocks();

    add_block(8);
    activate_blocks();
    clear_main_memory();
    clear_blocks();

    // After clearing, we should be able to start fresh
    CHECK(add_block(16), "add_block should work after clear");
    CHECK(activate_blocks(), "activate should work after clear");
    CHECK(get_block_by_index(0) != NULL, "block 0 should exist");

    clear_main_memory();
    clear_blocks();
    PASS();
}

// ----------------------------------------------------------
int main(void) {
    printf("\n=== main_memory tests ===\n\n");

    test_add_single_block();
    test_add_multiple_blocks();
    test_activate_and_get();
    test_double_activate_fails();
    test_get_out_of_bounds();
    test_clear_blocks_resets_state();

    printf("\n=== Results: %d/%d passed ===\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
