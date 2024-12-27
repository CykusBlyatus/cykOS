/**
 * @file csr.h
 * @author CykusBlyatus
 * @brief Contains multiple macros related to CSRs (Control/Status Registers)
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
#include "utils.h"

// Machine Status Register (mstatus)

#define CSR_MSTATUS_UIE     BIT(0) // User Interrupt Enable
#define CSR_MSTATUS_SIE     BIT(1) // Supervisor Interrupt Enable
#define CSR_MSTATUS_MIE     BIT(3) // Machine Interrupt Enable

#define CSR_MSTATUS_UPIE    BIT(4) // User Previous Interrupt Enable
#define CSR_MSTATUS_SPIE    BIT(5) // Supervisor Previous Interrupt Enable
#define CSR_MSTATUS_MPIE    BIT(7) // Machine Previous Interrupt Enable

#define CSR_MSTATUS_SPP     BIT(8) // Supervisor Previous Privilege (0 = user, 1 = supervisor)

#define CSR_MSTATUS_MPP   (3L << 11) // Machine Previous Privilege (0 = user, 1 = supervisor, 11 = machine)
#define CSR_MSTATUS_MPP_M (3L << 11) // previous privilege = machine
#define CSR_MSTATUS_MPP_S (1L << 11) // previous privilege = supervisor
#define CSR_MSTATUS_MPP_U          0 // previous privilege = user

#define CSR_MSTATUS_FS (3L << 13) // Floating-point Status (or something)
#define CSR_MSTATUS_XS (3L << 15) // eXtra Status (or something)

#define CSR_MSTATUS_MPRV    BIT(17) // Modify PRiVilege (if 1, memory access is restricted by the previous privilege, MPP)
#define CSR_MSTATUS_SUM     BIT(18) // Supervisor User Memory access (if 1, accesses to pages that are accessible by U-mode are permitted) 
#define CSR_MSTATUS_MXR     BIT(19) // Make eXecutable Readable (if 1, loads from pages marked executable will succeed)
#define CSR_MSTATUS_TVM     BIT(20) // Trap Virtual Memory
#define CSR_MSTATUS_TW      BIT(21) // Timeout Wait
#define CSR_MSTATUS_TSR     BIT(22) // Trap SRET

#define CSR_MSTATUS_UXL     (3L << 32) // User XLEN (1 = 32, 2 = 64, 3 = 128)
#define CSR_MSTATUS_SXL     (3L << 34) // Supervisor XLEN (1 = 32, 2 = 64, 3 = 128)

#define CSR_MSTATUS_SD      BIT(__riscv_xlen - 1) // State Dirty (summarizes whether either FS or XS field is dirty, being set to 1 if any is dirty)

/////////////////////////////////////////////////////

// Supervisor Status Register (sstatus)

#define CSR_SSTATUS_UIE     BIT(0) // User Interrupt Enable
#define CSR_SSTATUS_SIE     BIT(1) // Supervisor Interrupt Enable

#define CSR_SSTATUS_UPIE    BIT(4) // User Previous Interrupt Enable
#define CSR_SSTATUS_SPIE    BIT(5) // Supervisor Previous Interrupt Enable

#define CSR_SSTATUS_SPP     BIT(8) // Supervisor Previous Privilege (0 = user, 1 = supervisor)

#define CSR_SSTATUS_FS      (3L << 13) // Floating-point Status (or something)
#define CSR_SSTATUS_XS      (3L << 15) // eXtra Status (or something)

#define CSR_SSTATUS_SUM     BIT(18) // Supervisor User Memory access (if 1, accesses to pages that are accessible by U-mode are permitted) 
#define CSR_SSTATUS_MXR     BIT(19) // Make eXecutable Readable (if 1, loads from pages marked executable will succeed)

#define CSR_SSTATUS_UXL     (3L << 32) // User XLEN (1 = 32, 2 = 64, 3 = 128)

#define CSR_SSTATUS_SD      BIT(__riscv_xlen - 1) // State Dirty (summarizes whether either FS or XS field is dirty, being set to 1 if any is dirty)

/////////////////////////////////////////////////////

// Machine Trap-Vector Base-Address Register (mtvec)

#define CSR_MTVEC_MODE 3        // mask to get the mtvec mode (0 = direct (interrupts set pc to BASE), 1 = vectored (interrupts set pc to BASE+4*cause), 2+ = reserved)
#define CSR_MTVEC_DIRECT 0      // all traps set pc to mtvec
#define CSR_MTVEC_VECTORED 1    // interrupts set pc to mtvec + 4 * cause

/////////////////////////////////////////////////////

//  Machine Cause Register (mcause)

#define CSR_MCAUSE_INT(code) (BIT(__riscv_xlen - 1) | code) // merges the interrupt bit with the interrupt code

// Interrupt Codes

#define CSR_MCAUSE_USI CSR_MCAUSE_INT(0)  // User        Software Interrupt
#define CSR_MCAUSE_SSI CSR_MCAUSE_INT(1)  // Supervisor  Software Interrupt
//efine CSR_MCAUSE_HSI CSR_MCAUSE_INT(2)  // Hypervisor  Software Interrupt
#define CSR_MCAUSE_MSI CSR_MCAUSE_INT(3)  // Machine     Software Interrupt
#define CSR_MCAUSE_UTI CSR_MCAUSE_INT(4)  // User        Timer Interrupt
#define CSR_MCAUSE_STI CSR_MCAUSE_INT(5)  // Supervisor  Timer Interrupt
//efine CSR_MCAUSE_HTI CSR_MCAUSE_INT(6)  // Hypervisor  Timer Interrupt
#define CSR_MCAUSE_MTI CSR_MCAUSE_INT(7)  // Machine     Timer Interrupt
#define CSR_MCAUSE_UEI CSR_MCAUSE_INT(8)  // User        External Interrupt
#define CSR_MCAUSE_SEI CSR_MCAUSE_INT(9)  // Supervisor  External Interrupt
//efine CSR_MCAUSE_HEI CSR_MCAUSE_INT(10) // Hypervisor  External Interrupt
#define CSR_MCAUSE_MEI CSR_MCAUSE_INT(11) // Machine     External Interrupt

// Exception Codes

#define CSR_MCAUSE_INSTR_MISALIGN   0  // Instruction address misaligned
#define CSR_MCAUSE_INSTR_ACCESS     1  // Instruction access fault
#define CSR_MCAUSE_INSTR_ILLEGAL    2  // Illegal instruction
#define CSR_MCAUSE_BREAKPOINT       3  // Breakpoint
#define CSR_MCAUSE_LOAD_MISALIGN    4  // Load address misaligned
#define CSR_MCAUSE_LOAD_ACCESS      5  // Load access fault
#define CSR_MCAUSE_STORE_MISALIGN   6  // Store/AMO address misaligned
#define CSR_MCAUSE_STORE_ACCESS     7  // Store/AMO access fault
#define CSR_MCAUSE_ECALL_U          8  // Environment call from U-mode
#define CSR_MCAUSE_ECALL_S          9  // Environment call from S-mode
#define CSR_MCAUSE_ECALL_M          11 // Environment call from M-mode
#define CSR_MCAUSE_INSTR_PAGE       12 // Instruction page fault
#define CSR_MCAUSE_LOAD_PAGE        13 // Load page fault
#define CSR_MCAUSE_STORE_PAGE       15 // Store/AMO page fault

/////////////////////////////////////////////////////

// Machine Interrupt Enable (mie)

#define CSR_MIE_MEIE BIT(11) // Machine    External Interrupts Enable
//efine CSR_MIE_HEIE BIT(10) // Hypervisor External Interrupts Enable
#define CSR_MIE_SEIE BIT(9)  // Supervisor External Interrupts Enable
#define CSR_MIE_UEIE BIT(8)  // User       External Interrupts Enable
#define CSR_MIE_MTIE BIT(7)  // Machine    Timer Interrupts Enable
//efine CSR_MIE_HSIE BIT(6)  // Hypervisor Timer Interrupts Enable
#define CSR_MIE_STIE BIT(5)  // Supervisor Timer Interrupts Enable
#define CSR_MIE_UTIE BIT(4)  // User       Timer Interrupts Enable
#define CSR_MIE_MSIE BIT(3)  // Machine    Software Interrupts Enable
//efine CSR_MIE_HSIE BIT(2)  // Hypervisor Software Interrupts Enable
#define CSR_MIE_SSIE BIT(1)  // Supervisor Software Interrupts Enable
#define CSR_MIE_USIE BIT(0)  // User       Software Interrupts Enable

/////////////////////////////////////////////////////

// Supervisor Interrupt Enable (sie)
#define CSR_SIE_EXTERNAL    BIT(9)
#define CSR_SIE_TIMER       BIT(5)
#define CSR_SIE_SOFTWARE    BIT(1)

/////////////////////////////////////////////////////

// Machine Interrupt Pending (mip)

#define CSR_MIP_MEIP BIT(11) // Machine    External Interrupt Pending
//efine CSR_MIP_HEIP BIT(10) // Hypervisor External Interrupt Pending
#define CSR_MIP_SEIP BIT(9)  // Supervisor External Interrupt Pending
#define CSR_MIP_UEIP BIT(8)  // User       External Interrupt Pending
#define CSR_MIP_MTIP BIT(7)  // Machine    Timer Interrupt Pending
//efine CSR_MIP_HSIP BIT(6)  // Hypervisor Timer Interrupt Pending
#define CSR_MIP_STIP BIT(5)  // Supervisor Timer Interrupt Pending
#define CSR_MIP_UTIP BIT(4)  // User       Timer Interrupt Pending
#define CSR_MIP_MSIP BIT(3)  // Machine    Software Interrupt Pending
//efine CSR_MIP_HSIP BIT(2)  // Hypervisor Software Interrupt Pending
#define CSR_MIP_SSIP BIT(1)  // Supervisor Software Interrupt Pending
#define CSR_MIP_USIP BIT(0)  // User       Software Interrupt Pending

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
#define csrw(reg,val) do {\
    uint64_t x = (uint64_t) val;\
    DEBUG_INFO("csrw(\"" reg "\" " ANSI_MAGENTA "(%p)" ANSI_RESET ", %s " ANSI_MAGENTA "(%p)" ANSI_RESET ")", (void*)csrr_nodebug(reg), #val, (void*)x);\
    asm volatile ("csrw " reg ",%0" : : "r" (x));\
} while(0)

// CSR set bits
#define csrs(reg,bits) do {\
    uint64_t x = (uint64_t) bits;\
    DEBUG_INFO("csrs(\"" reg "\" " ANSI_MAGENTA "(%p)" ANSI_RESET ", %s " ANSI_MAGENTA "(%p)" ANSI_RESET ")", (void*)csrr_nodebug(reg), #bits, (void*)x);\
    asm volatile ("csrs " reg ",%0" : : "r" (x));\
} while(0)

// CSR clear bits
#define csrc(reg,bits) do {\
    uint64_t x = (uint64_t) bits;\
    DEBUG_INFO("csrc(\"" reg "\" " ANSI_MAGENTA "(%p)" ANSI_RESET ", %s " ANSI_MAGENTA "(%p)" ANSI_RESET ")", (void*)csrr_nodebug(reg), #bits, (void*)x);\
    asm volatile ("csrc " reg ",%0" : : "r" (x));\
} while(0)

// CSR read
#define csrr(reg) ({\
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
