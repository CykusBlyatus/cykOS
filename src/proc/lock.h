#ifndef CYKOS_LOCK_H
#define CYKOS_LOCK_H

#include <stdint.h>

typedef struct {
    u32 locked;
} spinlock_t;

void spinlock(spinlock_t*);
void spinrelease(spinlock_t*);

typedef struct {
    u32 locked;
} sleeplock_t;

void sleeplock(sleeplock_t*);
void sleeprelease(sleeplock_t*);

#endif
