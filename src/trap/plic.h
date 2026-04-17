#ifndef CYKOS_PLIC_H
#define CYKOS_PLIC_H

#include <stdint.h>

#define UART0_IRQ 10

#define PLIC_BASE 0x0C000000ull

void plic_set_priority(uint32_t irq, uint32_t priority);

void plic_enable_interrupt(uint32_t context, uint32_t irq);
void plic_disable_interrupt(uint32_t context, uint32_t irq);

void plic_set_threshold(uint32_t context, uint32_t threshold);

uint32_t plic_claim(uint32_t context);
void plic_complete(uint32_t context, uint32_t irq);

#endif // CYKOS_PLIC_H
