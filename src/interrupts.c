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
            DEBUG_INFO("external interrupt");
            uint32_t irq = plic_claim(HART_CONTEXT());
            switch (irq) {
                case UART0_IRQ:
                    DEBUG_INFO("UART interrupt");
                    break;
                default:
                    DEBUG_WARN("irq = %d", irq);
            }

            if (irq)
                plic_complete(HART_CONTEXT(), irq);

            register uint64_t a7 asm("a7") = ECALL_S_INT_ACK;
            register uint64_t a0 asm("a0") = CSR_MIP_SEIP;
            asm volatile ("ecall" : : "r" (a7), "r" (a0) : "memory");
            break;
        }
        case CSR_MCAUSE_STI: {
            DEBUG_INFO("timer interrupt");
            DEBUG_PRINTF("mtime = %p, mtimecmp = %p", (void*)mtime, (void*)mtimecmp);
            mtimecmp = mtime + 10000000;

            register uint64_t a7 asm("a7") = ECALL_S_INT_ACK;
            register uint64_t a0 asm("a0") = CSR_MIP_STIP;
            asm volatile ("ecall" : : "r" (a7), "r" (a0) : "memory");
            break;
        }
        case CSR_MCAUSE_SSI: {
            DEBUG_INFO("software interrupt (ignore for now)");

            register uint64_t a7 asm("a7") = ECALL_S_INT_ACK;
            register uint64_t a0 asm("a0") = CSR_MIP_SSIP;
            asm volatile ("ecall" : : "r" (a7), "r" (a0) : "memory");
            break;
        }
        default:
            DEBUG_WARN("scause = %p", (void*) scause);
    }
}
