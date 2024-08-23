/**
 * @file plic.c
 * @author CykusBlyatus
 * @brief Definition of functions for the PLIC (Platform-Level Interrupt Controller)
 * @see https://github.com/riscv/riscv-plic-spec/blob/master/riscv-plic.adoc
 */

#include "plic.h"

// Base address of the PLIC in QEMU's RISC-V `virt` machine
#define PLIC_BASE 0x0C000000ull

// Calculates the PLIC address at an offset specified in **bytes** (even though the returned value is a uint32_t*)
#define PLIC_AT(offset) ((volatile uint32_t*)(PLIC_BASE + offset))

// Assuming single hart (hart 0) for simplicity
#define HART_ID 0

void plic_set_priority(uint32_t irq, uint32_t priority) {
    *PLIC_AT(irq * 4) = priority;
}

void plic_enable_interrupt(uint32_t context, uint32_t irq) {
    // starts at plic + 0x2000
    // 1024 sources, one bit per -> 1024/8 = 128 (0x80) bytes to include all sources per context
    *PLIC_AT((0x2000) + (0x80 * context) + (4 * (irq / 32))) |= 1 << (irq % 32);
}

void plic_disable_interrupt(uint32_t context, uint32_t irq) {
    // same story as plic_enable_interrupt
    *PLIC_AT((0x2000) + (0x80 * context) + (4 * (irq / 32))) &= ~(1 << (irq % 32));
}

void plic_set_threshold(uint32_t context, uint32_t threshold) {
    *PLIC_AT(0x200000 + 0x1000 * context) = threshold;
}

uint32_t plic_claim(uint32_t context) {
    return *PLIC_AT(0x200004 + 0x1000 * context);
}

void plic_complete(uint32_t context, uint32_t irq) {
    *PLIC_AT(0x200004 + 0x1000 * context) = irq;
}
