#ifndef CYKOS_XLEN_H

// lazy way to make IDEs not complain
#ifndef __riscv
    #define __riscv
    #define __riscv_xlen 64
#endif

#if __riscv_xlen != 32 && __riscv_xlen != 64 && __riscv_xlen != 128
    #error "Unsupported RISC-V architecture (expected __riscv_xlen to be 32, 64 or 128)"
#endif

#endif
