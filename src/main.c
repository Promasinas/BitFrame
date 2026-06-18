#include "main_memory.h"
#include "log_init.h"
#include "log.h"
#include <stdio.h>

int main(void) {
    if (!log_init()) {
        fprintf(stderr, "ERROR: failed to initialize logging\n");
        return 1;
    }

    log_info("===== BitFream starting =====");

    /* ---- test: invalid allocation (not multiple of 8) ---- */
    log_info("Test 1: allocate 13 bytes (should fail)");
    if (!set_main_memory_block_size(13)) {
        log_warn("Test 1 passed: correctly rejected 13-byte allocation");
    }

    /* ---- test: valid allocation ---- */
    log_info("Test 2: allocate 256 bytes");
    if (set_main_memory_block_size(256)) {
        log_debug("Allocated %lu blocks at 0x%p", main_memory_block_size,
                  (void*)main_memory_block);
    }

    /* ---- test: double clear ---- */
    log_info("Test 3: clear memory (first time)");
    if (clear_main_memory_block()) {
        log_debug("First clear succeeded");
    }

    log_info("Test 4: clear memory (second time, should fail)");
    if (!clear_main_memory_block()) {
        log_warn("Test 4 passed: correctly rejected double-clear");
    }

    /* ---- test: re-allocate after clear ---- */
    log_info("Test 5: re-allocate 512 bytes");
    if (set_main_memory_block_size(512)) {
        log_trace("Re-allocation successful");
    }
    clear_main_memory_block();

    log_info("===== BitFream finished =====");

    log_shutdown();
    return 0;
}
