#include "lock.h"
#include <proc/thread.h>

void spinlock(lock_t *lock) {
    while (__sync_lock_test_and_set(&lock->locked, 1));
    // __sync_synchronize();
}

void sleeplock(lock_t *lock) {
    while (__sync_lock_test_and_set(&lock->locked, 1)) {
        sleep(lock);
    }
}

void release(lock_t *lock) {
    // __sync_synchronize();
    // lock->locked = 0;
    __sync_lock_release(&lock->locked);
    wakeup(lock);
}
