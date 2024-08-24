#ifndef CYKOS_TIMER_H
#define CYKOS_TIMER_H

#include <stdint.h>

#define mtimer_freq() 32768 /* [extremely loud incorrect buzzer] */

#define mtimer_sec_to_ticks(s) (s * mtimer_freq())
#define mtimer_ms_to_ticks(ms) (1000 * ms * mtimer_freq())
#define mtimer_us_to_ticks(us) (1000000 * us * mtimer_freq())

uint64_t *mtimecmp_();
#define mtimecmp (*mtimecmp_())

uint64_t mtime_();
#define mtime mtime_()

void mtimer_init();

#endif /* CYKOS_TIMER_H */
