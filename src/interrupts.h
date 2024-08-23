#ifndef CYKOS_INTERRUPTS_H
#define CYKOS_INTERRUPTS_H

// void __attribute__((interrupt ("machine"))) irq_handler();

void hw_enable_interrupts();

void kernel_trap();

#endif // CYKOS_INTERRUPTS_H
