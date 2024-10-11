#include "interrupts.h"
#include "plic.h"
#include "csr.h"
#include <stdint.h>

#include <auxiliary/debug_enable.h>
#include <auxiliary/debug.h>

#include "timer.h"
#include "syscon.h"
#include "uart.h"

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
                    char c = uart_read();
                    if (c == 27) // ESC
                        poweroff();
                    printf("Character received: '%c' (%d)\n", c, c);
                    break;
                default:
                    DEBUG_WARN("irq = %d", irq);
            }

            if (irq)
                plic_complete(HART_CONTEXT(), irq);

            break;
        }
        case CSR_MCAUSE_STI: {
            DEBUG_INFO("timer interrupt");
            uint64_t stimecmp = csrr("stimecmp");
            DEBUG_PRINTF("mtime = %p, stimecmp = %p", (void*)mtime, (void*)stimecmp);
            csrw("stimecmp", stimecmp + 10000000);
            break;
        }
        case CSR_MCAUSE_SSI: {
            DEBUG_INFO("software interrupt (ignore for now)");
            break;
        }
        default:
            DEBUG_WARN("scause = %p", (void*) scause);
            poweroff();
    }
}
