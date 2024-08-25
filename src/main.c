#include <stdio.h>
#include "uart.h"
#include "syscon.h"
#include "interrupts.h"
#include "plic.h"
#include "csr.h"
#include "timer.h"

extern void trap_handler_s();
extern void trap_handler_m();
int main();

void start() {
    csrw("mepc", main); // set "return" address to main
    csrw("mtvec", trap_handler_m);
    csrw("stvec", trap_handler_s);

    // set "previous" privilege mode and enable interrupts
    csrc("mstatus", CSR_MSTATUS_MPP);
    csrs("mstatus", CSR_MSTATUS_MPP_S | CSR_MSTATUS_SIE | CSR_MSTATUS_MIE);

    csrs("sstatus", CSR_SSTATUS_SIE);

    csrw("medeleg", 0xffff & ~BIT(CSR_MCAUSE_ECALL_S)); // delegate all exceptions (except ecall-S) to S-mode
    csrw("mideleg", CSR_MIP_SSIP | CSR_MIP_STIP | CSR_MIP_SEIP); // delegate all interrupts to S-mode
    csrs("mie", CSR_MIE_SSIE | CSR_MIE_STIE | CSR_MIE_MTIE | CSR_MIE_SEIE | CSR_MIE_MEIE);
    csrs("sie", CSR_SIE_SOFTWARE | CSR_SIE_TIMER | CSR_SIE_EXTERNAL);

    // configure Physical Memory Protection to give supervisor mode
    // access to all of physical memory.
    csrw("pmpaddr0", 0x3fffffffffffff);
    csrw("pmpcfg0", 0xf);
    csrw("satp", 0); // disable paging

    mtimer_init();

    DEBUG_INFO("calling mret");
    asm volatile ("mret"); // jump to to main
}

// (Already in Supervisor Mode)
int main() {
    printf("%s called\n", __func__);

    //*
    uart_init();
    plic_set_priority(UART0_IRQ, 1);
    plic_enable_interrupt(HART_CONTEXT(), UART0_IRQ);
    //*/

    DEBUG_SUCCESS("Entering uart_read() loop!");
    //*
    char c;
    while (1) {
        c = uart_read();
        if (c == 27) // ESC
            poweroff();
        putchar(c);
    }
    //*/

    /*
    while (1) {
        printf("Waiting for interrupt...\n");
        asm volatile ("wfi");
    }
    //*/

    DEBUG_WARN("reached end of main");
    poweroff();
}
