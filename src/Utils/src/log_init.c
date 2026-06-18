#include "log_init.h"
#include "log.h"
#include <stdio.h>
#include <time.h>

static FILE *log_file = NULL;

bool log_init(void) {
    /* build filename: logs/bitfream-YYYY-MM-DD-HH-MM-SS.log */
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char filename[64];
    snprintf(filename, sizeof(filename),
             "logs/bitfream-%04d-%02d-%02d-%02d-%02d-%02d.log",
             t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
             t->tm_hour, t->tm_min, t->tm_sec);

    log_file = fopen(filename, "w");
    if (log_file == NULL) {
        return false;
    }

    /* file: capture TRACE and above */
    log_add_fp(log_file, LOG_TRACE);

    /* console: keep stderr visible, TRACE and above */
    log_set_quiet(false);
    log_set_level(LOG_TRACE);

    log_info("---- log_init: logging to %s ----", filename);
    return true;
}

void log_shutdown(void) {
    log_info("---- log_shutdown: closing log file ----");
    if (log_file != NULL) {
        fclose(log_file);
        log_file = NULL;
    }
}
