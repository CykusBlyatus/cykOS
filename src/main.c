#include <stdio.h>
#include "uart.h"
#include "syscon.h"
#include "interrupts.h"
#include "plic.h"
#include "csr.h"
#include "timer.h"

extern void trap_handler();
extern void trap_handler_s();
extern void trap_handler_m();
int main();

void start() {
    csrw("mepc", main); // set "return" address to main
    csrw("mtvec", trap_handler_m); // set trap handler
    csrw("stvec", trap_handler_s);

    // set "previous" privilege mode
    DEBUG_CSRR("mstatus");
    csrc("mstatus", CSR_MSTATUS_MPP);
    csrs("mstatus", CSR_MSTATUS_MPP_S | CSR_MSTATUS_SIE | CSR_MSTATUS_MIE);
    DEBUG_CSRR("mstatus");

    DEBUG_CSRR("sstatus");
    csrs("sstatus", CSR_SSTATUS_SIE);
    DEBUG_CSRR("sstatus");

    // delegate all interrupts and exceptions to supervisor mode.
    csrw("medeleg", 0xffff);
    csrw("mideleg", 0xffff);
    csrs("mie", CSR_MIE_SEIE | CSR_MIE_STIE | CSR_MIE_SSIE | CSR_MIE_MTIE);
    csrs("sie", CSR_SIE_EXTERNAL | CSR_SIE_TIMER | CSR_SIE_SOFTWARE);

    // configure Physical Memory Protection to give supervisor mode
    // access to all of physical memory.
    csrw("pmpaddr0", 0x3fffffffffffff);
    csrw("pmpcfg0", 0xf);
    csrw("satp", 0); // disable paging

    DEBUG_CSRR("mepc");

    mtimer_init();

    DEBUG_INFO("calling mret");
    asm volatile ("mret"); // jump to to main
}

int main() {
    printf("%s called\n", __func__);
    uart_init();
    plic_set_priority(UART0_IRQ, 1);
    plic_enable_interrupt(HART_CONTEXT(), UART0_IRQ);

    int x = 5;
    printf("among piss (x = %d and &x = %p)\n", x, &x);

    /*
    char c;
    while (1) {
        c = uart_read();
        if (c == 27) // ESC
            break;
        putchar(c);
    }
    //*/

    //*
    while (1) {
        printf("Waiting for interrupt...\n");
        asm volatile ("wfi");
    }
    //*/

    poweroff();
}
