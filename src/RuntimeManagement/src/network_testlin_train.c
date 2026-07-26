#include "network_testlin_train.h"

// ================================================================
// Static callback array — fixed size, cache-friendly, O(1) access
// ================================================================
typedef struct {
    bf_train_cb_t callback;
    void*         context;
} bf_train_entry_t;

static bf_train_entry_t train_entries[BF_TRAIN_MAX_CALLBACKS];
static size_t           train_count = 0;

// ================================================================
bool bf_train_register(bf_train_cb_t callback, void* context)
{
    if (callback == NULL) {
        return false;
    }
    if (train_count >= BF_TRAIN_MAX_CALLBACKS) {
        return false;
    }

    train_entries[train_count].callback = callback;
    train_entries[train_count].context  = context;
    train_count++;
    return true;
}

// ================================================================
bool bf_train_execute(void)
{
    for (size_t i = 0; i < train_count; i++) {
        if (!train_entries[i].callback(train_entries[i].context)) {
            return false;   // abort chain on first failure
        }
    }
    return true;
}

// ================================================================
void bf_train_clear(void)
{
    train_count = 0;
}

// ================================================================
size_t bf_train_count(void)
{
    return train_count;
}
