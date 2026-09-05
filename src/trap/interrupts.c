#include <auxiliary/debug_enable.h>
#include <auxiliary/debug.h>

#include "interrupts.h"
#include <memory/paging.h>
#include <devices/virtio/blk.h>
#include "plic.h"
#include <include/riscv.h>
#include <stdint.h>
#include <stdio.h>
#include <proc/thread.h>
#include <include/riscv.h>
#include <devices/timer/timer.h>
#include <devices/uart/uart.h>
#include <devices/uart/uart_macros.h>
#include <devices/syscon/syscon.h>

void kernel_trap(cpucontext_t *ctx) {
    DEBUG_INFO("called");

    for (int i = 0; i < 31; ++i)
        DEBUG_INFO("x%d (%s) = %p (%llu)", i+1, regnames[i+1], (void*)ctx->regs[i], ctx->regs[i]);

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
                case DISK0_IRQ: {
                    virtio_blk_intr();
                    break;
                }
                default:
                    DEBUG_WARN("Unhandled IRQ %d", irq);
            }

            if (irq)
                plic_complete(HART_CONTEXT(), irq);

            break;
        }
        case CSR_CAUSE_STI: {
            DEBUG_INFO("timer interrupt");
            CSRR("stimecmp");
            CSRW("stimecmp", mtime + SCHED_QUANTUM);
            ctx->pc = (void*)CSRR("sepc");
            CSRS("sstatus", CSR_STATUS_SIE);
            sched(ctx);
            CSRW("sepc", ctx->pc);
            break;
        }
        case CSR_CAUSE_ECALL_S: { // yield()
            CSRW("stimecmp", mtime + SCHED_QUANTUM);
            ctx->pc = (void*)CSRR("sepc") + 4;
            DEBUG_INFO("ctx->pc = %p", ctx->pc);
            CSRS("sstatus", CSR_STATUS_SIE);
            sched(ctx);
            CSRW("sepc", ctx->pc);
            break;
        }
        case CSR_CAUSE_INSTR_ILLEGAL: {
            // first float operation triggers illegal instruction, need to check if that's the case
            if ((CSRR("sstatus") & CSR_STATUS_FS) != CSR_STATUS_FS_INIT)
                panic("Illegal instruction at %p", (void*)CSRR("sepc"));
            // zero-initialize float registers before first use for security and determinism
            asm volatile (
                "fmv.d.x f0,x0\n"
                "fmv.d f1,f0\n"
                "fmv.d f2,f0\n"
                "fmv.d f3,f0\n"
                "fmv.d f4,f0\n"
                "fmv.d f5,f0\n"
                "fmv.d f6,f0\n"
                "fmv.d f7,f0\n"
                "fmv.d f8,f0\n"
                "fmv.d f9,f0\n"
                "fmv.d f10,f0\n"
                "fmv.d f11,f0\n"
                "fmv.d f12,f0\n"
                "fmv.d f13,f0\n"
                "fmv.d f14,f0\n"
                "fmv.d f15,f0\n"
                "fmv.d f16,f0\n"
                "fmv.d f17,f0\n"
                "fmv.d f18,f0\n"
                "fmv.d f19,f0\n"
                "fmv.d f20,f0\n"
                "fmv.d f21,f0\n"
                "fmv.d f22,f0\n"
                "fmv.d f23,f0\n"
                "fmv.d f24,f0\n"
                "fmv.d f25,f0\n"
                "fmv.d f26,f0\n"
                "fmv.d f27,f0\n"
                "fmv.d f28,f0\n"
                "fmv.d f29,f0\n"
                "fmv.d f30,f0\n"
                "fmv.d f31,f0\n"
                );
                // CSRW("sstatus", (sstatus & ~CSR_STATUS_FS) | CSR_STATUS_FS_CLEAN);
                CSRS("sstatus", CSR_STATUS_FS_DIRTY);
            break;
        }
        case CSR_CAUSE_LOAD_PAGE:
        case CSR_CAUSE_STORE_PAGE: {
            void *addr = (void*)CSRR("stval");
            // map heap pages on demand
            if (addr >= (void*)KERNEL_HEAP_START && addr < (void*)KERNEL_HEAP_END) {
                u64 pa = pgalloc();
                void *va = (void*)((u64)addr & ~(PGSIZE - 1)); // align VA with page
                pgmap(
                    &kernel_pgdir,
                    pa,
                    va,
                    PTE_RW
                );
            } else {
                panic("%s Page Fault at %p", scause == CSR_CAUSE_LOAD_PAGE ? "Load" : "Store", addr);
            }
            break;
        }
        case CSR_CAUSE_INSTR_PAGE:
            panic("Instruction Page Fault at %p", (void*)CSRR("stval"));
            break;
        default:
            panic("scause = %p\n", (void*) scause);
    }
}
