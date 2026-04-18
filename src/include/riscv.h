#ifndef CYKOS_RISCV_H
#define CYKOS_RISCV_H

#include <stdint.h>
#include "xlen.h"

#if __rsicv_xlen == 32
    typedef u32 cpureg_t;
#elif __riscv_xlen == 64
    typedef u64 cpureg_t;
#elif __riscv_xlen == 128
    typedef u128 cpureg_t;
#else
    #error Expected __riscv_xlen to be 32, 64 or 128
#endif

typedef struct {
    union {
        struct {
            cpureg_t x1;
            cpureg_t x2;
            cpureg_t x3;
            cpureg_t x4;
            cpureg_t x5;
            cpureg_t x6;
            cpureg_t x7;
            cpureg_t x8;
            cpureg_t x9;
            cpureg_t x10;
            cpureg_t x11;
            cpureg_t x12;
            cpureg_t x13;
            cpureg_t x14;
            cpureg_t x15;
            cpureg_t x16;
            cpureg_t x17;
            cpureg_t x18;
            cpureg_t x19;
            cpureg_t x20;
            cpureg_t x21;
            cpureg_t x22;
            cpureg_t x23;
            cpureg_t x24;
            cpureg_t x25;
            cpureg_t x26;
            cpureg_t x27;
            cpureg_t x28;
            cpureg_t x29;
            cpureg_t x30;
            cpureg_t x31;
        };
        struct {
            cpureg_t ra;
            cpureg_t sp;
            cpureg_t gp;
            cpureg_t tp;
            cpureg_t t0;
            cpureg_t t1;
            cpureg_t t2;
            cpureg_t s0;
            cpureg_t s1;
            cpureg_t a0;
            cpureg_t a1;
            cpureg_t a2;
            cpureg_t a3;
            cpureg_t a4;
            cpureg_t a5;
            cpureg_t a6;
            cpureg_t a7;
            cpureg_t s2;
            cpureg_t s3;
            cpureg_t s4;
            cpureg_t s5;
            cpureg_t s6;
            cpureg_t s7;
            cpureg_t s8;
            cpureg_t s9;
            cpureg_t s10;
            cpureg_t s11;
            cpureg_t t3;
            cpureg_t t4;
            cpureg_t t5;
            cpureg_t t6;
        };
        cpureg_t regs[31];
    };
    void *pc; // program counter
} cpucontext_t;

extern const char * const regnames[32];

#endif
