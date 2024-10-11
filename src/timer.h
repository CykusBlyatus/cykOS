#ifndef CYKOS_TIMER_H
#define CYKOS_TIMER_H

#include <stdint.h>

uint64_t *mtimecmp_();
#define mtimecmp (*mtimecmp_())

uint64_t mtime_();
#define mtime mtime_()

void mtimer_init();
void stimer_init();

#endif /* CYKOS_TIMER_H */
