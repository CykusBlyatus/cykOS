#ifndef CYKOS_PLIC_H
#define CYKOS_PLIC_H

#include <stdint.h>

void plic_set_priority(uint32_t irq, uint32_t priority);
void plic_enable_interrupt(uint32_t irq);
uint32_t plic_claim(void);
void plic_complete(uint32_t irq);

#endif // CYKOS_PLIC_H
