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
    DEBUG_PRINTF("called");

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
                    while (!(UART0_IIR & UART_IIR_NO_INT)) {
                        switch (UART0_IIR & 0x7) {
                            case UART_IIR_RDA: {
                                char c = uart_read();
                                if (c == 27) // ESC
                                    poweroff();
                                DEBUG_PRINTF("Received character %d\n", (int)c);
                                break;
                            }
                            default:
                                DEBUG_WARN("UART0_IIR = %p\n", (void*)(uint64_t)UART0_IIR);
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
            DEBUG_PRINTF("mtime = %p, stimecmp = %p", (void*)mtime, (void*)stimecmp);
            CSRW("stimecmp", stimecmp + 10000000);
            break;
        }
        case CSR_CAUSE_SSI: {
            DEBUG_INFO("software interrupt (ignore for now)");
            break;
        }
        default:
            DEBUG_WARN("scause = %p", (void*) scause);
            poweroff();
    }
}
