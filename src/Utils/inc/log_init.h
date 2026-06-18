#ifndef __LOG_INIT_H__
#define __LOG_INIT_H__

#include <stdbool.h>

/**
 * Initialize the logging subsystem.
 *   Opens logs/bitfream.log for file output (TRACE level).
 *   Console (stderr) output remains enabled.
 *   Returns true on success, false if the log file cannot be opened.
 */
bool log_init(void);

/**
 * Shut down the logging subsystem.
 *   Closes the log file handle.
 */
void log_shutdown(void);

#endif
