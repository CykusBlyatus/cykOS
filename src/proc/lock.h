#ifndef CYKOS_LOCK_H
#define CYKOS_LOCK_H

#include <stdint.h>

typedef struct {
    u8 locked;
} lock_t;

void spinlock(lock_t*);
void sleeplock(lock_t*);
void release(lock_t*);

#endif
