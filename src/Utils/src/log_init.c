#include "log_init.h"
#include "log.h"
#include <stdio.h>

static FILE *log_file = NULL;

bool log_init(void) {
    log_file = fopen("logs/bitfream.log", "w");
    if (log_file == NULL) {
        return false;
    }

    /* file: capture TRACE and above */
    log_add_fp(log_file, LOG_TRACE);

    /* console: keep stderr visible, TRACE and above */
    log_set_quiet(false);
    log_set_level(LOG_TRACE);

    log_info("---- log_init: logging to logs/bitfream.log ----");
    return true;
}

void log_shutdown(void) {
    log_info("---- log_shutdown: closing log file ----");
    if (log_file != NULL) {
        fclose(log_file);
        log_file = NULL;
    }
}
