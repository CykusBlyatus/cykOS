#ifndef CYKOS_TIMER_H
#define CYKOS_TIMER_H

#include <stdint.h>

#define MTIMER_ADDR 0x2000000

#define mtimecmp_ ((uint64_t*)(MTIMER_ADDR + 0x4000))
#define mtimecmp (*mtimecmp_)

#define mtime_ ((uint64_t*)(MTIMER_ADDR + 0xBFF8))
#define mtime (*mtime_)

void mtimer_init();
void stimer_init();

#endif /* CYKOS_TIMER_H */
