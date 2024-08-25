// UNUSED FILE

#ifndef CYKOS_RISCV_REGS_H
#define CYKOS_RISCV_REGS_H

#include <stdint.h>

// lazy way to make IDEs not complain
#ifndef __riscv
    #define __riscv
    #define __riscv_xlen 64
#endif

#if __riscv_xlen == 32
    typedef uint32_t cpureg_t;
#elif __riscv_xlen == 64
    typedef uint64_t cpureg_t;
#elif __riscv_xlen == 128
    typedef uint128_t cpureg_t;
#else
    #error "Unsuported __riscv_xlen (must be 32, 64 or 128)"
#endif

#define reg_r(reg) ({\
    cpureg_t x;\
    asm volatile ("mv %0," reg : "=r" (x));\
    x;\
})

#define reg_w(reg, val) {\
    cpureg_t x = (cpureg_t) val;\
    asm volatile ("mv " reg ",%0" : : "r" (x));\
}

/**
 * @brief Container for all RISC-V general-purpose registers
 */
typedef struct riscv_cpu {
    // union {cpureg_t x0, zero;};
    union {cpureg_t x1, ra;};
    union {cpureg_t x2, sp;};
    union {cpureg_t x3, gp;};
    union {cpureg_t x4, tp;};
    union {cpureg_t x5, t0;};
    union {cpureg_t x6, t1;};
    union {cpureg_t x7, t2;};
    union {cpureg_t x8, s0, fp;};
    union {cpureg_t x9, s1;};
    union {cpureg_t x10, a0;};
    union {cpureg_t x11, a1;};
    union {cpureg_t x12, a2;};
    union {cpureg_t x13, a3;};
    union {cpureg_t x14, a4;};
    union {cpureg_t x15, a5;};
    union {cpureg_t x16, a6;};
    union {cpureg_t x17, a7;};
    union {cpureg_t x18, s2;};
    union {cpureg_t x19, s3;};
    union {cpureg_t x20, s4;};
    union {cpureg_t x21, s5;};
    union {cpureg_t x22, s6;};
    union {cpureg_t x23, s7;};
    union {cpureg_t x24, s8;};
    union {cpureg_t x25, s9;};
    union {cpureg_t x26, s10;};
    union {cpureg_t x27, s11;};
    union {cpureg_t x28, t3;};
    union {cpureg_t x29, t4;};
    union {cpureg_t x30, t5;};
    union {cpureg_t x31, t6;};
} riscv_cpu_t;

/**
 * @brief Pushes all general-purpose register values to the stack
 */
#define riscv_cpu_save() {\
    asm volatile("addi sp, sp, %0" : : "i" (-32 * sizeof(cpureg_t)));\
    register riscv_cpu_t *cpu asm("sp");\
    cpu->x1 = reg_r("x1");\
    cpu->x2 = reg_r("x2");\
    cpu->x3 = reg_r("x3");\
    cpu->x4 = reg_r("x4");\
    cpu->x5 = reg_r("x5");\
    cpu->x6 = reg_r("x6");\
    cpu->x7 = reg_r("x7");\
    cpu->x8 = reg_r("x8");\
    cpu->x9 = reg_r("x9");\
    cpu->x10 = reg_r("x10");\
    cpu->x11 = reg_r("x11");\
    cpu->x12 = reg_r("x12");\
    cpu->x13 = reg_r("x13");\
    cpu->x14 = reg_r("x14");\
    cpu->x15 = reg_r("x15");\
    cpu->x16 = reg_r("x16");\
    cpu->x17 = reg_r("x17");\
    cpu->x18 = reg_r("x18");\
    cpu->x19 = reg_r("x19");\
    cpu->x20 = reg_r("x20");\
    cpu->x21 = reg_r("x21");\
    cpu->x22 = reg_r("x22");\
    cpu->x23 = reg_r("x23");\
    cpu->x24 = reg_r("x24");\
    cpu->x25 = reg_r("x25");\
    cpu->x26 = reg_r("x26");\
    cpu->x27 = reg_r("x27");\
    cpu->x28 = reg_r("x28");\
    cpu->x29 = reg_r("x29");\
    cpu->x30 = reg_r("x30");\
    cpu->x31 = reg_r("x31");\
}

/**
 * @brief Loads/pops all (previously saved) general-purpose register values from the stack
 */
#define riscv_cpu_restore() {\
    register riscv_cpu_t *cpu asm("sp");\
    reg_w("x1", cpu->x1);\
    reg_w("x2", cpu->x2);\
    reg_w("x3", cpu->x3);\
    reg_w("x4", cpu->x4);\
    reg_w("x5", cpu->x5);\
    reg_w("x6", cpu->x6);\
    reg_w("x7", cpu->x7);\
    reg_w("x8", cpu->x8);\
    reg_w("x9", cpu->x9);\
    reg_w("x10", cpu->x10);\
    reg_w("x11", cpu->x11);\
    reg_w("x12", cpu->x12);\
    reg_w("x13", cpu->x13);\
    reg_w("x14", cpu->x14);\
    reg_w("x15", cpu->x15);\
    reg_w("x16", cpu->x16);\
    reg_w("x17", cpu->x17);\
    reg_w("x18", cpu->x18);\
    reg_w("x19", cpu->x19);\
    reg_w("x20", cpu->x20);\
    reg_w("x21", cpu->x21);\
    reg_w("x22", cpu->x22);\
    reg_w("x23", cpu->x23);\
    reg_w("x24", cpu->x24);\
    reg_w("x25", cpu->x25);\
    reg_w("x26", cpu->x26);\
    reg_w("x27", cpu->x27);\
    reg_w("x28", cpu->x28);\
    reg_w("x29", cpu->x29);\
    reg_w("x30", cpu->x30);\
    reg_w("x31", cpu->x31);\
    asm volatile ("addi sp, sp, %0" : : "i" (32 * sizeof(cpureg_t)));\
}

#endif /* CYKOS_RISCV_REGS_H */
