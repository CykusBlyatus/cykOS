/**
 * @file csr.h
 * @author CykusBlyatus
 * @brief Contains multiple macros related to CSRs (Control/Status Registers)
 * @see https://docs.riscv.org/reference/isa/_attachments/riscv-privileged.pdf
 */

#ifndef CYKOS_CSR_H
#define CYKOS_CSR_H

#define HART_CONTEXT() 0 // TODO: change it if you're ready to do multi-core :p

// lazy way to make IDEs not complain
#ifndef __riscv
    #define __riscv
    #define __riscv_xlen 64
#endif

#if __riscv_xlen != 32 && __riscv_xlen != 64 && __riscv_xlen != 128
    #error "Unsupported RISC-V architecture (expected __riscv_xlen to be 32, 64 or 128)"
#endif

#include <stdint.h>

// Status Register (mstatus, sstatus, etc.)

#define CSR_STATUS_UIE     (1L << 0) // User Interrupt Enable
#define CSR_STATUS_SIE     (1L << 1) // Supervisor Interrupt Enable
#define CSR_STATUS_MIE     (1L << 3) // Machine Interrupt Enable

#define CSR_STATUS_UPIE    (1L << 4) // User Previous Interrupt Enable
#define CSR_STATUS_SPIE    (1L << 5) // Supervisor Previous Interrupt Enable
#define CSR_STATUS_MPIE    (1L << 7) // Machine Previous Interrupt Enable

#define CSR_STATUS_SPP     (1L << 8) // Supervisor Previous Privilege (0 = user, 1 = supervisor)

#define CSR_STATUS_MPP   (3L << 11) // Machine Previous Privilege (0 = user, 1 = supervisor, 11 = machine)
#define CSR_STATUS_MPP_M (3L << 11) // previous privilege = machine
#define CSR_STATUS_MPP_S (1L << 11) // previous privilege = supervisor
#define CSR_STATUS_MPP_U          0 // previous privilege = user

#define CSR_STATUS_FS (3L << 13) // Floating-point Status (or something)
#define CSR_STATUS_XS (3L << 15) // eXtra Status (or something)

#define CSR_STATUS_MPRV    (1L << 17) // Modify PRiVilege (if 1, memory access is restricted by the previous privilege, MPP)
#define CSR_STATUS_SUM     (1L << 18) // Supervisor User Memory access (if 1, accesses to pages that are accessible by U-mode are permitted)
#define CSR_STATUS_MXR     (1L << 19) // Make eXecutable Readable (if 1, loads from pages marked executable will succeed)
#define CSR_STATUS_TVM     (1L << 20) // Trap Virtual Memory
#define CSR_STATUS_TW      (1L << 21) // Timeout Wait
#define CSR_STATUS_TSR     (1L << 22) // Trap SRET

#define CSR_STATUS_UXL     (3L << 32) // User XLEN (1 = 32, 2 = 64, 3 = 128)
#define CSR_STATUS_SXL     (3L << 34) // Supervisor XLEN (1 = 32, 2 = 64, 3 = 128)

#define CSR_STATUS_SD      (1L << (__riscv_xlen - 1)) // State Dirty (summarizes whether either FS or XS field is dirty, being set to 1 if any is dirty)

/////////////////////////////////////////////////////

// Trap-Vector Base-Address Register (mtvec, stvec, etc.)

#define CSR_TVEC_MODE_MASK 3   // mask to get the tvec mode
#define CSR_TVEC_DIRECT 0      // all traps set pc to tvec
#define CSR_TVEC_VECTORED 1    // interrupts set pc to tvec + 4 * cause

/////////////////////////////////////////////////////

// Interrupt/Exception Cause Register (mcause, scause, etc.)

#define CSR_CAUSE_INT(code) ((1L << (__riscv_xlen - 1)) | (code)) // merges the interrupt bit with the interrupt code

// Interrupt Codes

#define CSR_CAUSE_USI CSR_CAUSE_INT(0)  // User        Software Interrupt
#define CSR_CAUSE_SSI CSR_CAUSE_INT(1)  // Supervisor  Software Interrupt
#define CSR_CAUSE_HSI CSR_CAUSE_INT(2)  // Hypervisor  Software Interrupt
#define CSR_CAUSE_MSI CSR_CAUSE_INT(3)  // Machine     Software Interrupt
#define CSR_CAUSE_UTI CSR_CAUSE_INT(4)  // User        Timer Interrupt
#define CSR_CAUSE_STI CSR_CAUSE_INT(5)  // Supervisor  Timer Interrupt
#define CSR_CAUSE_HTI CSR_CAUSE_INT(6)  // Hypervisor  Timer Interrupt
#define CSR_CAUSE_MTI CSR_CAUSE_INT(7)  // Machine     Timer Interrupt
#define CSR_CAUSE_UEI CSR_CAUSE_INT(8)  // User        External Interrupt
#define CSR_CAUSE_SEI CSR_CAUSE_INT(9)  // Supervisor  External Interrupt
#define CSR_CAUSE_HEI CSR_CAUSE_INT(10) // Hypervisor  External Interrupt
#define CSR_CAUSE_MEI CSR_CAUSE_INT(11) // Machine     External Interrupt

// Exception Codes

#define CSR_CAUSE_INSTR_MISALIGN   0  // Instruction address misaligned
#define CSR_CAUSE_INSTR_ACCESS     1  // Instruction access fault
#define CSR_CAUSE_INSTR_ILLEGAL    2  // Illegal instruction
#define CSR_CAUSE_BREAKPOINT       3  // Breakpoint
#define CSR_CAUSE_LOAD_MISALIGN    4  // Load address misaligned
#define CSR_CAUSE_LOAD_ACCESS      5  // Load access fault
#define CSR_CAUSE_STORE_MISALIGN   6  // Store/AMO address misaligned
#define CSR_CAUSE_STORE_ACCESS     7  // Store/AMO access fault
#define CSR_CAUSE_ECALL_U          8  // Environment call from U-mode
#define CSR_CAUSE_ECALL_S          9  // Environment call from S-mode
#define CSR_CAUSE_ECALL_H          10 // Environment call from H-mode
#define CSR_CAUSE_ECALL_M          11 // Environment call from M-mode
#define CSR_CAUSE_INSTR_PAGE       12 // Instruction page fault
#define CSR_CAUSE_LOAD_PAGE        13 // Load page fault
#define CSR_CAUSE_STORE_PAGE       15 // Store/AMO page fault

/////////////////////////////////////////////////////

// Interrupt Enable/Pending (mie/mip, sie/sip, etc.)

#define CSR_IEIP_MEI (1L << 11) // Machine    External Interrupts
#define CSR_IEIP_HEI (1L << 10) // Hypervisor External Interrupts
#define CSR_IEIP_SEI (1L << 9)  // Supervisor External Interrupts
#define CSR_IEIP_UEI (1L << 8)  // User       External Interrupts
#define CSR_IEIP_MTI (1L << 7)  // Machine    Timer Interrupts
#define CSR_IEIP_HTI (1L << 6)  // Hypervisor Timer Interrupts
#define CSR_IEIP_STI (1L << 5)  // Supervisor Timer Interrupts
#define CSR_IEIP_UTI (1L << 4)  // User       Timer Interrupts
#define CSR_IEIP_MSI (1L << 3)  // Machine    Software Interrupts
#define CSR_IEIP_HSI (1L << 2)  // Hypervisor Software Interrupts
#define CSR_IEIP_SSI (1L << 1)  // Supervisor Software Interrupts
#define CSR_IEIP_USI (1L << 0)  // User       Software Interrupts

/////////////////////////////////////////////////////



// Supervisor Address Translation and Protection (satp)

#define CSR_SATP_BARE 0 // No paging
#if __riscv_xlen == 32
    #define CSR_SATP_PPN(satp) ((satp) & 0x3fffff) // Physical Page Number (Physical Address divided by PGSIZE)
    #define CSR_SATP_ASID(satp) ((satp) & (0x1ff << 22)) // Address Space ID
    #define CSR_SATP_MODE(satp) ((satp) & (1 << 31)) // Paging Mode
    #define CSR_SATP_SV32 (1ull << 31)  // Page-based 32-bit virtual addressing
#elif __riscv_xlen == 64 || __riscv_xlen == 128
    #define CSR_SATP_PPN(satp) ((satp) & 0xfffffffffff) // Physical Page Number (Physical Address divided by PGSIZE)
    #define CSR_SATP_ASID(satp) ((satp) & (0xffff << 44)) // Address Space ID
    #define CSR_SATP_MODE(satp) ((satp) & (0xf << 60)) // Paging Mode
    #define CSR_SATP_SV39 (8ull << 60)  // Page-based 39-bit virtual addressing
    #define CSR_SATP_SV48 (9ull << 60)  // Page-based 48-bit virtual addressing
    #define CSR_SATP_SV57 (10ull << 60) // Page-based 57-bit virtual addressing
    #define CSR_SATP_SV64 (11ull << 60) // Reserved for page-based 64-bit virtual addressing
#endif

/////////////////////////////////////////////////////

// Control/Status Register (CSR) instructions

// Since this is a header file, we have to make sure it doesn't affect other files' DEBUG
#ifdef DEBUG
    #define DEBUG_DEFINED
#endif

#define DEBUG_CSR // For now, CSR debug is enabled everywhere

// macro that can be set if includer wants to receive CSR debug info
#ifdef DEBUG_CSR
    #define DEBUG
#else
    #undef DEBUG
#endif

#include <auxiliary/debug.h>

// CSR write
#define CSRW(reg,val) do {\
    uint64_t x = (uint64_t) val;\
    DEBUG_INFO("csrw(\"" reg "\" " ANSI_MAGENTA "(%p)" ANSI_RESET ", %s " ANSI_MAGENTA "(%p)" ANSI_RESET ")", (void*)csrr_nodebug(reg), #val, (void*)x);\
    asm volatile ("csrw " reg ",%0" : : "r" (x));\
} while(0)

// CSR set bits
#define CSRS(reg,bits) do {\
    uint64_t x = (uint64_t) bits;\
    DEBUG_INFO("csrs(\"" reg "\" " ANSI_MAGENTA "(%p)" ANSI_RESET ", %s " ANSI_MAGENTA "(%p)" ANSI_RESET ")", (void*)csrr_nodebug(reg), #bits, (void*)x);\
    asm volatile ("csrs " reg ",%0" : : "r" (x));\
} while(0)

// CSR clear bits
#define CSRC(reg,bits) do {\
    uint64_t x = (uint64_t) bits;\
    DEBUG_INFO("csrc(\"" reg "\" " ANSI_MAGENTA "(%p)" ANSI_RESET ", %s " ANSI_MAGENTA "(%p)" ANSI_RESET ")", (void*)csrr_nodebug(reg), #bits, (void*)x);\
    asm volatile ("csrc " reg ",%0" : : "r" (x));\
} while(0)

// CSR read
#define CSRR(reg) ({\
    uint64_t x;\
    asm volatile ("csrr %0," reg : "=r" (x));\
    DEBUG_INFO("csrr(\"%s\") " ANSI_MAGENTA "(%p)" ANSI_RESET, reg, (void*)x);\
    x;\
})

// Force csrr to not print debug info
// Used by csrw, csrs and csrc when printing their own debug info
#define csrr_nodebug(reg) ({\
    uint64_t x;\
    asm volatile ("csrr %0," reg : "=r" (x));\
    x;\
})

// Restore includer's DEBUG flag
#ifdef DEBUG_DEFINED
    #define DEBUG
#else
    #undef DEBUG
#endif
#undef DEBUG_DEFINED

#endif /* CYKOS_CSR_H */
