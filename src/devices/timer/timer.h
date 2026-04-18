#ifndef CYKOS_TIMER_H
#define CYKOS_TIMER_H

#include <stdint.h>

#define MTIMER_ADDR 0x2000000

#define mtimecmp_ ((u64*)(MTIMER_ADDR + 0x4000))
#define mtimecmp (*mtimecmp_)

#define mtime_ ((u64*)(MTIMER_ADDR + 0xBFF8))
#define mtime (*mtime_)

#define SCHED_QUANTUM 10000000

#endif /* CYKOS_TIMER_H */
