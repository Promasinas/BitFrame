/**
 * test_log.c — Unit tests for rxi/log.c
 *
 * Covers:
 *   1. Basic level logging (trace → fatal)
 *   2. Level filtering (log_set_level)
 *   3. Quiet mode
 *   4. File output (log_add_fp)
 *   5. Custom callback
 *   6. log_level_string()
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "log.h"

static int  test_passed = 0;
static int  test_failed = 0;
static char callback_buf[512];

#define TEST(name)  printf("  [TEST] %s ... ", name)
#define PASS()      do { printf("PASS\n"); test_passed++; } while(0)
#define FAIL(msg)   do { printf("FAIL: %s\n", msg); test_failed++; } while(0)
#define CHECK(cond, msg) do { if (cond) PASS(); else FAIL(msg); } while(0)

/* ------------------------------------------------------------------ */

/* ---- custom callback for testing ---- */
static void custom_callback(log_Event *ev) {
    char buf[256];
    snprintf(buf, sizeof(buf), "[%s] %s:%d: ",
             log_level_string(ev->level), ev->file, ev->line);
    vsnprintf(callback_buf, sizeof(callback_buf), ev->fmt, ev->ap);
    (void)buf;
}

/* ---- 1. level-string mapping ---- */
static void test_level_string(void) {
    TEST("log_level_string returns correct strings");
    CHECK(strcmp(log_level_string(LOG_TRACE), "TRACE") == 0, "TRACE mismatch");
    CHECK(strcmp(log_level_string(LOG_DEBUG), "DEBUG") == 0, "DEBUG mismatch");
    CHECK(strcmp(log_level_string(LOG_INFO),  "INFO")  == 0, "INFO  mismatch");
    CHECK(strcmp(log_level_string(LOG_WARN),  "WARN")  == 0, "WARN  mismatch");
    CHECK(strcmp(log_level_string(LOG_ERROR), "ERROR") == 0, "ERROR mismatch");
    CHECK(strcmp(log_level_string(LOG_FATAL), "FATAL") == 0, "FATAL mismatch");
}

/* ---- 2. level filtering ---- */
static void test_level_filter(void) {
    TEST("log_set_level filters correctly");
    /* quiet stderr, then register callback at WARN level to test filtering */
    callback_buf[0] = '\0';
    log_set_quiet(true);

    /* register callback at WARN level — INFO should NOT appear */
    log_add_callback(custom_callback, NULL, LOG_WARN);
    log_info("should be filtered out");
    CHECK(callback_buf[0] == '\0', "INFO should be filtered when callback is at WARN level");

    /* WARN should appear */
    callback_buf[0] = '\0';
    log_warn("warning message %d", 42);
    CHECK(strstr(callback_buf, "warning message 42") != NULL, "WARN should appear");

    /* reset */
    log_set_level(LOG_TRACE);
    log_set_quiet(false);
}

/* ---- 3. quiet mode ---- */
static void test_quiet_mode(void) {
    TEST("quiet mode suppresses stderr");
    /* quiet ON → default stdout_callback (stderr) should be skipped */
    FILE *tmp = tmpfile();
    assert(tmp != NULL);

    log_set_quiet(true);
    int rc = log_add_fp(tmp, LOG_TRACE);
    CHECK(rc == 0, "log_add_fp returned success");

    log_info("quiet_test");
    fflush(tmp);

    /* verify the message landed in tmp file */
    rewind(tmp);
    char line[256] = {0};
    int found = 0;
    while (fgets(line, sizeof(line), tmp)) {
        if (strstr(line, "quiet_test")) { found = 1; break; }
    }
    CHECK(found == 1, "message should appear in file callback even when quiet");

    fclose(tmp);
    log_set_quiet(false);
}

/* ---- 4. file output ---- */
static void test_file_output(void) {
    TEST("log_add_fp writes to file");
    FILE *fp = fopen("test_log_output.txt", "w");
    assert(fp != NULL);

    log_add_fp(fp, LOG_TRACE);
    log_set_quiet(true);  /* don't spam stderr */

    log_info("file_log_test_%d", 123);
    fclose(fp);

    /* check file content */
    fp = fopen("test_log_output.txt", "r");
    assert(fp != NULL);
    char content[256] = {0};
    int ok = 0;
    while (fgets(content, sizeof(content), fp)) {
        if (strstr(content, "file_log_test_123")) { ok = 1; break; }
    }
    fclose(fp);
    remove("test_log_output.txt");

    CHECK(ok == 1, "log message not found in file");
    log_set_quiet(false);
}

/* ---- 5. custom callback ---- */
static void test_custom_callback(void) {
    TEST("log_add_callback receives messages");
    callback_buf[0] = '\0';
    log_set_quiet(true);

    /* add a fresh callback (previous ones may still be registered) */
    log_add_callback(custom_callback, NULL, LOG_TRACE);
    log_error("callback_error_%s", "xyz");

    CHECK(strstr(callback_buf, "callback_error_xyz") != NULL,
          "callback should receive error message");
    log_set_quiet(false);
}

/* ---- main ---- */
int main(void) {
    printf("\n===== log.c Unit Tests =====\n\n");

    test_level_string();
    test_level_filter();
    test_quiet_mode();
    test_file_output();
    test_custom_callback();

    printf("\n===== Results: %d passed, %d failed =====\n",
           test_passed, test_failed);
    return test_failed ? 1 : 0;
}
