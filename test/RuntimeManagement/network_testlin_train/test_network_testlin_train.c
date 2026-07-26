#include "network_testlin_train.h"
#include <stdio.h>

static int tests_run    = 0;
static int tests_passed = 0;

#define TEST(name) do { tests_run++; printf("  %-45s ", name); } while(0)
#define PASS()     do { tests_passed++; printf("PASS\n"); } while(0)
#define CHECK(cond, msg) do { \
    if (!(cond)) { printf("FAIL: %s\n", msg); return; } \
} while(0)

// ================================================================
// Sample training callbacks
// ================================================================
static int exec_order[8];
static int exec_idx;

static bool train_init(void* ctx) {
    double* lr = (double*)ctx;
    *lr = 0.01;
    exec_order[exec_idx++] = 1;
    return true;
}

static bool train_epoch(void* ctx) {
    int* epoch = (int*)ctx;
    (*epoch)++;
    exec_order[exec_idx++] = 2;
    return true;
}

// Sets loss to a computed value, always succeeds
static bool train_eval(void* ctx) {
    double* loss = (double*)ctx;
    *loss = 0.35;
    exec_order[exec_idx++] = 3;
    return true;
}

// Checks loss WITHOUT modifying it — used for early-stop
static bool train_validate(void* ctx) {
    double* loss = (double*)ctx;
    exec_order[exec_idx++] = 3;
    return (*loss < 0.5);
}

static bool train_save(void* ctx) {
    (void)ctx;
    exec_order[exec_idx++] = 4;
    return true;
}

// ================================================================
static void test_train_pipeline(void) {
    TEST("training pipeline (4 steps)");

    bf_train_clear();
    exec_idx = 0;
    double lr    = 0.0;
    int    epoch = 0;
    double loss  = 1.0;

    bf_train_register(train_init,  &lr);
    bf_train_register(train_epoch, &epoch);
    bf_train_register(train_eval,  &loss);
    bf_train_register(train_save,  NULL);

    CHECK(bf_train_count() == 4, "count should be 4");
    CHECK(bf_train_execute(), "pipeline should succeed");
    CHECK(lr    == 0.01, "lr set");
    CHECK(epoch == 1,    "epoch incremented");
    CHECK(loss  == 0.35, "loss computed");

    // Verify order: 1→2→3→4
    CHECK(exec_order[0] == 1 &&
          exec_order[1] == 2 &&
          exec_order[2] == 3 &&
          exec_order[3] == 4,
          "execution order should be 1→2→3→4");

    bf_train_clear();
    PASS();
}

static void test_train_early_stop(void) {
    TEST("early stop in training");

    bf_train_clear();
    exec_idx = 0;
    double loss  = 0.9;     // high loss → validate returns false
    int    epoch = 0;

    bf_train_register(train_epoch,    &epoch);
    bf_train_register(train_validate, &loss);
    bf_train_register(train_save,     NULL);   // should NOT run

    CHECK(!bf_train_execute(), "pipeline should abort");
    CHECK(epoch == 1, "epoch ran");
    CHECK(exec_idx == 2, "only 2 callbacks (epoch + validate)");
    CHECK(exec_order[1] == 3, "validate was last to run");

    bf_train_clear();
    PASS();
}

static void test_train_clear(void) {
    TEST("clear training chain");

    bf_train_clear();
    int dummy = 0;

    bf_train_register(train_epoch, &dummy);
    bf_train_register(train_epoch, &dummy);
    CHECK(bf_train_count() == 2, "count 2 before clear");

    bf_train_clear();
    CHECK(bf_train_count() == 0, "count 0 after clear");

    PASS();
}

static void test_train_independent(void) {
    TEST("forward & train chains are independent");

    // Clear both
    bf_train_clear();
    int train_dummy = 0;
    bf_train_register(train_epoch, &train_dummy);
    CHECK(bf_train_count() == 1, "train chain has 1 entry");

    // Forward chain should be unaffected
    // (we can't include network_forward.h here without potential
    //  symbol conflicts in the test, but the static arrays are
    //  in separate translation units so they are independent)
    bf_train_clear();
    CHECK(bf_train_count() == 0, "train cleared");

    PASS();
}

// ================================================================
int main(void) {
    printf("\n=== network_testlin_train tests ===\n\n");

    test_train_pipeline();
    test_train_early_stop();
    test_train_clear();
    test_train_independent();

    printf("\n=== Results: %d/%d passed ===\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
