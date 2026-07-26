#ifndef __LOG_INIT_H__
#define __LOG_INIT_H__

#include <stdbool.h>

// --- DLL export / import ----------------------------------------
#ifndef BF_UTILS_API
  #if defined(_WIN32)
    #ifdef BF_UTILS_EXPORTS
      #define BF_UTILS_API __declspec(dllexport)
    #else
      #define BF_UTILS_API __declspec(dllimport)
    #endif
  #else
    #define BF_UTILS_API __attribute__((visibility("default")))
  #endif
#endif

BF_UTILS_API bool log_init(void);
BF_UTILS_API void log_shutdown(void);

#endif
