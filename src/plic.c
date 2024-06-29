#include "plic.h"

// Base address of the PLIC in QEMU's RISC-V `virt` machine
#define PLIC_BASE_ADDR 0x0C000000ull

// Offsets for PLIC registers (these are typical and might vary)
#define PLIC_PRIORITY_OFFSET   0x0000
#define PLIC_ENABLE_OFFSET     0x2000
#define PLIC_THRESHOLD_OFFSET  0x200000
#define PLIC_CLAIM_OFFSET      0x200004

// Helper macros to calculate addresses
#define PLIC_PRIORITY(irq)     (PLIC_BASE_ADDR + PLIC_PRIORITY_OFFSET + (irq) * 4)
#define PLIC_ENABLE(hart)      (PLIC_BASE_ADDR + PLIC_ENABLE_OFFSET + (hart) * 0x80)
#define PLIC_THRESHOLD(hart)   (PLIC_BASE_ADDR + PLIC_THRESHOLD_OFFSET + (hart) * 0x1000)
#define PLIC_CLAIM(hart)       (PLIC_BASE_ADDR + PLIC_CLAIM_OFFSET + (hart) * 0x1000)

// Assuming single hart (hart 0) for simplicity
#define HART_ID 0

void plic_set_priority(uint32_t irq, uint32_t priority) {
    *(volatile uint32_t*)(PLIC_PRIORITY(irq)) = priority;
}

void plic_enable_interrupt(uint32_t irq) {
    // Each hart has its own enable register, enabling interrupt for hart 0
    *(volatile uint32_t *)(PLIC_ENABLE(HART_ID) + (irq / 32) * 4) |= (1 << (irq % 32));
}

uint32_t plic_claim(void) {
    return *(volatile uint32_t *)(PLIC_CLAIM(HART_ID));
}

void plic_complete(uint32_t irq) {
    *(volatile uint32_t *)(PLIC_CLAIM(HART_ID)) = irq;
}
