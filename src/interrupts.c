#include "interrupts.h"
#include "plic.h"
#include "uart.h"
#include "syscon.h"
#include "csr.h"
#include <stdint.h>
#include <stdio.h>

#include <auxiliary/debug_enable.h>
#include <auxiliary/debug.h>

/*
void __attribute__((interrupt ("machine"))) irq_handler() {
    uint32_t irq = plic_claim(HART_CONTEXT());
    if (irq == UART0_IRQ) {
        char c = uart_read();
        if (c == 27) // ESC
            poweroff();
        putchar(c);
    }
    plic_complete(HART_CONTEXT(), irq);
}
*/

extern void trap_handler();

void hw_enable_interrupts() {
    // Enable global interrupts in the machine status register
    asm volatile ("csrw mtvec, %0" :: "r" (trap_handler)); // Set trap handler
    asm volatile ("csrw mstatus, 8\n"); // Set MIE (Machine Interrupt Enable) bit

    /*
    // Enable external interrupts in the machine interrupt enable register
    asm volatile (
        "li t0, 0x800\n"       // Load the immediate value 0x800 (bit 11)
        "csrrs zero, mie, t0\n"  // Set the MEIE (Machine External Interrupt Enable) bit
    );
    //*/
}

void kernel_trap() {
    DEBUG_PRINTF("called");

    uint64_t sepc, sstatus, scause;
    sepc = csrr("sepc");
    csrr("mepc");
    sstatus = csrr("sstatus");
    csrr("mstatus");
    scause = csrr("scause");
    csrr("mcause");

    csrr("sip");
    csrr("mip");

    switch (scause) {
        case CSR_MCAUSE_SEI: {
            int irq = plic_claim(HART_CONTEXT());
            switch (irq) {
                case UART0_IRQ:
                    DEBUG_SUCCESS("UART interrupt");
                    break;
                default:
                    DEBUG_WARN("Interrupt not from UART (irq = %d)", irq);
            }

            if (irq)
                plic_complete(HART_CONTEXT(), irq);

            break;
        }
        default:
            DEBUG_WARN("scause = %p", (void*) scause);
    }

    csrw("sepc", sepc);
    csrw("sstatus", sstatus);

    // poweroff();
}
