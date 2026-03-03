#include "interrupts.h"
#include "plic.h"
#include "csr.h"
#include <stdint.h>

#include <auxiliary/debug_enable.h>
#include <auxiliary/debug.h>

#include "timer.h"
#include "syscon.h"
#include "uart.h"
#include "uart_macros.h"

void kernel_trap() {
    DEBUG_INFO("called");

    __attribute__((unused)) uint64_t
        sepc = CSRR("sepc"),
        sstatus = CSRR("sstatus"),
        scause = CSRR("scause"),
        sip = CSRR("sip");

    switch (scause) {
        case CSR_CAUSE_SEI: {
            DEBUG_INFO("external interrupt");
            uint32_t irq = plic_claim(HART_CONTEXT());
            switch (irq) {
                case UART0_IRQ: {
                    DEBUG_INFO("UART0 interrupt");
                    while (1) {
                        uint8_t iir = UART0_IIR;
                        if (iir & UART_IIR_NO_INT)
                            break;

                        switch (iir & 0x7) {
                            case UART_IIR_RDA: {
                                char c = uart_read();
                                if (c == 27) // ESC
                                    poweroff();
                                DEBUG_INFO("Received character %d\n", (int)c);
                                break;
                            }
                            default:
                                DEBUG_WARN("UART0_IIR = %p\n", (void*)(uint64_t)iir);
                        }
                    }
                    break;
                }
                default:
                    DEBUG_WARN("irq = %d", irq);
            }

            if (irq)
                plic_complete(HART_CONTEXT(), irq);

            break;
        }
        case CSR_CAUSE_STI: {
            DEBUG_INFO("timer interrupt");
            uint64_t stimecmp = CSRR("stimecmp");
            DEBUG_INFO("mtime = %p, stimecmp = %p", (void*)mtime, (void*)stimecmp);
            CSRW("stimecmp", stimecmp + 10000000);
            break;
        }
        case CSR_CAUSE_SSI: {
            DEBUG_ERROR("ecall handling not implemented");
            poweroff();
        }
        case CSR_CAUSE_INSTR_ILLEGAL: {
            // first float operation triggers illegal instruction, need to check if that's the case
            if ((CSRR("sstatus") & CSR_STATUS_FS) != CSR_STATUS_FS_INIT) {
                DEBUG_ERROR("Illegal instruction");
                poweroff();
            }
            // zero-initialize float registers before first use for security and determinism
            asm volatile (
                "fmv.d.x f0,x0;\
                fmv.d f1,f0;\
                fmv.d f2,f0;\
                fmv.d f3,f0;\
                fmv.d f4,f0;\
                fmv.d f5,f0;\
                fmv.d f6,f0;\
                fmv.d f7,f0;\
                fmv.d f8,f0;\
                fmv.d f9,f0;\
                fmv.d f10,f0;\
                fmv.d f11,f0;\
                fmv.d f12,f0;\
                fmv.d f13,f0;\
                fmv.d f14,f0;\
                fmv.d f15,f0;\
                fmv.d f16,f0;\
                fmv.d f17,f0;\
                fmv.d f18,f0;\
                fmv.d f19,f0;\
                fmv.d f20,f0;\
                fmv.d f21,f0;\
                fmv.d f22,f0;\
                fmv.d f23,f0;\
                fmv.d f24,f0;\
                fmv.d f25,f0;\
                fmv.d f26,f0;\
                fmv.d f27,f0;\
                fmv.d f28,f0;\
                fmv.d f29,f0;\
                fmv.d f30,f0;\
                fmv.d f31,f0;\
                ");
                // CSRW("sstatus", (sstatus & ~CSR_STATUS_FS) | CSR_STATUS_FS_CLEAN);
                CSRS("sstatus", CSR_STATUS_FS_DIRTY);
            break;
        }
        default:
            DEBUG_ERROR("scause = %p\n", (void*) scause);
            poweroff();
    }
}
