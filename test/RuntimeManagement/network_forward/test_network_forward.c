#include "network_forward.h"
#include <stdio.h>

static int tests_run    = 0;
static int tests_passed = 0;

#define TEST(name) do { tests_run++; printf("  %-45s ", name); } while(0)
#define PASS()     do { tests_passed++; printf("PASS\n"); } while(0)
#define CHECK(cond, msg) do { \
    if (!(cond)) { printf("FAIL: %s\n", msg); return; } \
} while(0)

// ================================================================
// Sample callbacks for testing
// ================================================================
static int exec_order[8];
static int exec_idx;

static bool cb_a(void* ctx) {
    int* counter = (int*)ctx;
    exec_order[exec_idx++] = 'A';
    (*counter)++;
    return true;
}

static bool cb_b(void* ctx) {
    int* counter = (int*)ctx;
    exec_order[exec_idx++] = 'B';
    (*counter)++;
    return true;
}

static bool cb_stop(void* ctx) {
    (void)ctx;
    exec_order[exec_idx++] = 'X';
    return false;   // abort chain
}

static bool cb_never(void* ctx) {
    (void)ctx;
    exec_order[exec_idx++] = 'N';
    return true;
}

// ================================================================
static void test_register_and_execute(void) {
    TEST("register 2 callbacks + execute");

    bf_forward_clear();
    exec_idx = 0;
    int counter = 0;

    CHECK(bf_forward_register(cb_a, &counter), "register cb_a");
    CHECK(bf_forward_register(cb_b, &counter), "register cb_b");
    CHECK(bf_forward_count() == 2, "count should be 2");
    CHECK(bf_forward_execute(), "execute should succeed");
    CHECK(counter == 2, "both callbacks should run");
    CHECK(exec_order[0] == 'A' && exec_order[1] == 'B',
          "execution order should be A→B");

    bf_forward_clear();
    PASS();
}

static void test_abort_chain(void) {
    TEST("abort chain on first failure");

    bf_forward_clear();
    exec_idx = 0;
    int counter = 0;

    bf_forward_register(cb_a, &counter);
    bf_forward_register(cb_stop, NULL);     // returns false
    bf_forward_register(cb_never, NULL);    // should NOT run

    CHECK(!bf_forward_execute(), "execute should return false");
    CHECK(counter == 1, "only first callback should run");
    CHECK(exec_order[0] == 'A' && exec_order[1] == 'X',
          "order should be A→X, N should not appear");
    CHECK(exec_idx == 2, "exactly 2 callbacks executed");

    bf_forward_clear();
    PASS();
}

static void test_null_rejected(void) {
    TEST("NULL callback rejected");

    bf_forward_clear();
    CHECK(!bf_forward_register(NULL, NULL), "NULL callback should be rejected");
    CHECK(bf_forward_count() == 0, "count should be 0");

    bf_forward_clear();
    PASS();
}

static void test_max_capacity(void) {
    TEST("max capacity (128)");

    bf_forward_clear();
    int dummy = 0;

    for (int i = 0; i < BF_FORWARD_MAX_CALLBACKS; i++) {
        CHECK(bf_forward_register(cb_a, &dummy), "should accept within limit");
    }
    CHECK(bf_forward_count() == BF_FORWARD_MAX_CALLBACKS, "count at max");

    // One more should fail
    CHECK(!bf_forward_register(cb_a, &dummy), "should reject when full");
    CHECK(bf_forward_count() == BF_FORWARD_MAX_CALLBACKS, "count unchanged");

    bf_forward_clear();
    PASS();
}

static void test_clear(void) {
    TEST("clear resets state");

    bf_forward_clear();
    int dummy = 0;

    bf_forward_register(cb_a, &dummy);
    bf_forward_register(cb_b, &dummy);
    CHECK(bf_forward_count() == 2, "count 2 before clear");

    bf_forward_clear();
    CHECK(bf_forward_count() == 0, "count 0 after clear");
    CHECK(bf_forward_execute(), "execute on empty chain should succeed");

    PASS();
}

// ================================================================
int main(void) {
    printf("\n=== network_forward tests ===\n\n");

    test_register_and_execute();
    test_abort_chain();
    test_null_rejected();
    test_max_capacity();
    test_clear();

    printf("\n=== Results: %d/%d passed ===\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
