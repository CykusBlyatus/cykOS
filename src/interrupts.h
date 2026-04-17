#ifndef CYKOS_INTERRUPTS_H
#define CYKOS_INTERRUPTS_H

#include <riscv.h>

void kernel_trap(cpucontext_t *ctx);

#endif // CYKOS_INTERRUPTS_H
