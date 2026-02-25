#include <stdio.h>
#include "uart.h"
#include "syscon.h"
#include "plic.h"
#include "csr.h"
#include "timer.h"

extern void trap_handler_s();
int main();

void start() {
    CSRW("mepc", main); // set "return" address to main
    CSRW("stvec", trap_handler_s);

    // set "previous" privilege mode and enable interrupts
    CSRC("mstatus", CSR_STATUS_MPP);
    CSRS("mstatus", CSR_STATUS_MPP_S);

    CSRS("sstatus", CSR_STATUS_SIE);

    CSRW("medeleg", 0xffff); // delegate all exceptions to S-mode
    CSRW("mideleg", 0xffff); // delegate all interrupts to S-mode
    CSRS("mie", CSR_IEIP_SSI | CSR_IEIP_STI | CSR_IEIP_SEI); // enable all S-mode interrupts
    CSRS("sie", CSR_IEIP_SSI | CSR_IEIP_STI | CSR_IEIP_SEI); // enable all S-mode interrupts

    // Configure physical memory protection to give supervisor mode access to all of physical memory.
    CSRW("pmpaddr0", 0x3fffffffffffff);
    CSRW("pmpcfg0", 0xf);
    CSRW("satp", 0); // disable paging

    stimer_init();

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

    /*
    DEBUG_SUCCESS("Entering uart_read() loop!");
    char c;
    while (1) {
        c = uart_read();
        if (c == 27) // ESC
            poweroff();
        putchar(c);
    }
    //*/

    //*
    DEBUG_SUCCESS("Entering 'wfi' loop!");
    while (1) {
        printf("Waiting for interrupt...\n");
        asm volatile ("wfi");
    }
    //*/

    DEBUG_WARN("reached end of main");
    poweroff();
}
