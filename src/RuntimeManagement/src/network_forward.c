#include "network_forward.h"

// ================================================================
// Static callback array — fixed size, cache-friendly, O(1) access
// ================================================================
typedef struct {
    bf_forward_cb_t callback;
    void*           context;
} bf_forward_entry_t;

static bf_forward_entry_t forward_entries[BF_FORWARD_MAX_CALLBACKS];
static size_t             forward_count = 0;

// ================================================================
bool bf_forward_register(bf_forward_cb_t callback, void* context)
{
    if (callback == NULL) {
        return false;
    }
    if (forward_count >= BF_FORWARD_MAX_CALLBACKS) {
        return false;
    }

    forward_entries[forward_count].callback = callback;
    forward_entries[forward_count].context  = context;
    forward_count++;
    return true;
}

// ================================================================
bool bf_forward_execute(void)
{
    for (size_t i = 0; i < forward_count; i++) {
        if (!forward_entries[i].callback(forward_entries[i].context)) {
            return false;   // abort chain on first failure
        }
    }
    return true;
}

// ================================================================
void bf_forward_clear(void)
{
    forward_count = 0;
}

// ================================================================
size_t bf_forward_count(void)
{
    return forward_count;
}
