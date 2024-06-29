#ifndef CYKOS_INTERRUPTS_H
#define CYKOS_INTERRUPTS_H

void __attribute__((interrupt)) irq_handler();

void hw_enable_interrupts();

#define UART0_IRQ 10

#endif // CYKOS_INTERRUPTS_H
