#include "interrupts.h"
#include "plic.h"
#include "uart.h"
#include "syscon.h"
#include "csr.h"
#include "riscv_regs.h"
#include "ecall_s.h"
#include <stdint.h>
#include <stdio.h>

#include <auxiliary/debug_enable.h>
#include <auxiliary/debug.h>

#include "timer.h"

void kernel_trap() {
    DEBUG_PRINTF("called");

    __attribute__((unused)) uint64_t
        sepc = csrr("sepc"),
        sstatus = csrr("sstatus"),
        scause = csrr("scause"),
        sip = csrr("sip");

    switch (scause) {
        case CSR_MCAUSE_SEI: {
            int irq = plic_claim(HART_CONTEXT());
            switch (irq) {
                case UART0_IRQ:
                    DEBUG_INFO("UART interrupt");
                    break;
                default:
                    DEBUG_WARN("irq = %d", irq);
            }

            if (irq)
                plic_complete(HART_CONTEXT(), irq);

            break;
        }
        case CSR_MCAUSE_STI:
            DEBUG_INFO("timer interrupt");
            DEBUG_PRINTF("mtime = %p, mtimecmp = %p", (void*)mtime, (void*)mtimecmp);
            mtimecmp = mtime + 10000000;
            register uint64_t a7 asm("a7") = ECALL_S_STI_ACK;
            asm volatile ("ecall" : : "r" (a7));
            break;
        case CSR_MCAUSE_SSI:
            DEBUG_INFO("software interrupt (ignore for now)");
            break;
        case CSR_MCAUSE_ECALL_S:
            DEBUG_INFO("ecall from S-Mode");
            csrw("sepc", csrr("sepc") + 4);
            csrr("sepc");
            break;
        default:
            DEBUG_WARN("scause = %p", (void*) scause);
    }
}

/*
void machine_trap() {
    DEBUG_PRINTF("called");

    __attribute__((unused)) uint64_t
        mepc = csrr("mepc"),
        mstatus = csrr("mstatus"),
        mcause = csrr("mcause"),
        mip = csrr("mip");
    
    if (mcause & BIT(__riscv_xlen - 1)) { // Interrupt -> delegate to supervisor
        csrc("mie", mip);
        csrs("mip", mip >> 2);
        return;
    }

    DEBUG_INFO("mcause = %p", (void*) mcause);
    csrc("mip", CSR_MIP_STIP);
    csrw("mepc", csrr("mepc") + 4);
}
*/
