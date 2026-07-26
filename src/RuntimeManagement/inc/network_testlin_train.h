#ifndef __NETWORK_TESTLIN_TRAIN_H__
#define __NETWORK_TESTLIN_TRAIN_H__

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
// Training callback chain
// ================================================================
// Callbacks are stored in a static array and executed in
// registration order.  Each callback receives a void* context
// and returns true on success / false to abort the chain.
// ================================================================

#define BF_TRAIN_MAX_CALLBACKS 128

typedef bool (*bf_train_cb_t)(void* context);

// Register a training callback + context.  Returns false if full.
BF_RUNTIME_API bool bf_train_register(bf_train_cb_t callback,
                                      void*         context);

// Execute all registered training callbacks in registration order.
// Stops early if any callback returns false.
// Returns true if all callbacks succeed.
BF_RUNTIME_API bool bf_train_execute(void);

// Remove all registered training callbacks.
BF_RUNTIME_API void bf_train_clear(void);

// Return the number of registered training callbacks.
BF_RUNTIME_API size_t bf_train_count(void);

#endif
