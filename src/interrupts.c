#include "interrupts.h"
#include "plic.h"
#include "uart.h"
#include <stdint.h>
#include <stdio.h>

void __attribute__((interrupt)) irq_handler() {
    uint32_t irq = plic_claim();
    if (irq == UART0_IRQ) {
        char c = uart_read();
        putchar(c);
    }
    plic_complete(irq);
}

void hw_enable_interrupts() {
    // Enable global interrupts in the machine status register
    asm volatile ("csrsi mstatus, 8"); // Set MIE (Machine Interrupt Enable) bit
    // Enable external interrupts in the machine interrupt enable register
    asm volatile (
        "li t0, 0x800\n"       // Load the immediate value 0x800 (bit 11)
        "csrrs zero, mie, t0"  // Set the MEIE (Machine External Interrupt Enable) bit
    );
}
