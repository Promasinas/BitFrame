#ifndef __NETWORK_FORWARD_H__
#define __NETWORK_FORWARD_H__

#include <stdbool.h>
#include <stddef.h>

// --- DLL export / import ----------------------------------------
#ifndef BF_RUNTIME_API
  #if defined(_WIN32)
    #ifdef BF_RUNTIME_EXPORTS
      #define BF_RUNTIME_API __declspec(dllexport)
    #else
      #define BF_RUNTIME_API __declspec(dllimport)
    #endif
  #else
    #define BF_RUNTIME_API __attribute__((visibility("default")))
  #endif
#endif

// ================================================================
// Forward-pass callback chain
// ================================================================
// Callbacks are stored in a static array and executed in
// registration order.  Each callback receives a void* context
// and returns true on success / false to abort the chain.
// ================================================================

#define BF_FORWARD_MAX_CALLBACKS 128

typedef bool (*bf_forward_cb_t)(void* context);

// Register a callback + context.  Returns false if the array is full.
BF_RUNTIME_API bool bf_forward_register(bf_forward_cb_t callback,
                                        void*          context);

// Execute all registered callbacks in registration order.
// Stops early if any callback returns false.
// Returns true if all callbacks succeed.
BF_RUNTIME_API bool bf_forward_execute(void);

// Remove all registered callbacks.
BF_RUNTIME_API void bf_forward_clear(void);

// Return the number of registered callbacks.
BF_RUNTIME_API size_t bf_forward_count(void);

#endif
