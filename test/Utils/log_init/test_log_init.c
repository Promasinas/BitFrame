#include "log_init.h"
#include "log.h"
#include <stdio.h>

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) do { tests_run++; printf("  %-45s ", name); } while(0)
#define PASS()     do { tests_passed++; printf("PASS\n"); } while(0)
#define FAIL(msg)  do { printf("FAIL: %s\n", msg); return; } while(0)
#define CHECK(cond, msg) do { if (!(cond)) { FAIL(msg); } } while(0)

// ----------------------------------------------------------
// log_init  creates logs/bitfream-<timestamp>.log
// log_shutdown closes it.
// ----------------------------------------------------------

static void test_init_and_shutdown(void) {
    TEST("log_init + log_shutdown");

    // First shutdown any previously open file (idempotent by design)
    log_shutdown();

    CHECK(log_init(), "log_init should succeed (creates log file)");

    // Write a message so the file has content
    log_info("test message from test_log_init");

    log_shutdown();
    PASS();
}

static void test_double_init(void) {
    TEST("double log_init (opens new file)");

    log_shutdown();
    CHECK(log_init(), "first log_init should succeed");
    log_shutdown();

    // Second init should also work (creates a new timestamped file)
    CHECK(log_init(), "second log_init should succeed");
    log_shutdown();
    PASS();
}

// ----------------------------------------------------------
int main(void) {
    printf("\n=== log_init tests ===\n\n");

    test_init_and_shutdown();
    test_double_init();

    printf("\n=== Results: %d/%d passed ===\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
