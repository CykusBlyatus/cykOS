/**
 * @file csr.h
 * @author CykusBlyatus
 * @brief Contains multiple macros related to CSRs (Control/Status Registers)
 */

#ifndef CYKOS_CSR_H
#define CYKOS_CSR_H

#define HART_CONTEXT() 1 // TODO: change it if you're ready to do multi-core :p

#define XLEN 64 // RISCV-64, this macro should be moved somewhere else in the future

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

#define CSR_MSTATUS_SD      BIT(XLEN - 1) // State Dirty (summarizes whether either FS or XS field is dirty, being set to 1 if any is dirty)

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

#define CSR_SSTATUS_SD      BIT(XLEN - 1) // State Dirty (summarizes whether either FS or XS field is dirty, being set to 1 if any is dirty)

/////////////////////////////////////////////////////

// Machine Trap-Vector Base-Address Register (mtvec)

#define CSR_MTVEC_ADDR_TO_BASE(addr) (addr << 2) // shifts the address to the right position
#define CSR_MTVEC_MODE 3 // mask to get the mtvec mode (0 = direct (interrupts set pc to BASE), 1 = vectored (interrupts set pc to BASE+4*cause), 2+ = reserved)
#define CSR_MTVEC_GET_ADDR(mtvec) (mtvec >> 2) // gets the address stored in mtvec

/////////////////////////////////////////////////////

//  Machine Cause Register (mcause)

#define CSR_MCAUSE_INT(code) (BIT(XLEN - 1) | code) // merges the interrupt bit with the interrupt code

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

#define CSR_MIE_MEIE BIT(11) // (Machine) Machine    External Interrupts Enable
//efine CSR_MIE_HEIE BIT(10) // (Machine) Hypervisor External Interrupts Enable
#define CSR_MIE_SEIE BIT(9)  // (Machine) Supervisor External Interrupts Enable
#define CSR_MIE_UEIE BIT(8)  // (Machine) User       External Interrupts Enable
#define CSR_MIE_MTIE BIT(7)  // (Machine) Machine    Timer Interrupts Enable
//efine CSR_MIE_HSIE BIT(6)  // (Machine) Hypervisor Timer Interrupts Enable
#define CSR_MIE_STIE BIT(5)  // (Machine) Supervisor Timer Interrupts Enable
#define CSR_MIE_UTIE BIT(4)  // (Machine) User       Timer Interrupts Enable
#define CSR_MIE_MSIE BIT(3)  // (Machine) Machine    Software Interrupts Enable
//efine CSR_MIE_HSIE BIT(2)  // (Machine) Hypervisor Software Interrupts Enable
#define CSR_MIE_SSIE BIT(1)  // (Machine) Supervisor Software Interrupts Enable
#define CSR_MIE_USIE BIT(0)  // (Machine) User       Software Interrupts Enable

/////////////////////////////////////////////////////

// Supervisor Interrupt Enable (sie)
#define CSR_SIE_EXTERNAL    BIT(9)
#define CSR_SIE_TIMER       BIT(5)
#define CSR_SIE_SOFTWARE    BIT(1)

/////////////////////////////////////////////////////

// Control/Status Register (CSR) instructions

#include <auxiliary/debug_enable.h>
#include <auxiliary/debug.h>

// CSR write
#define csrw(reg,val) do {\
    uint64_t x = (uint64_t) val;\
    DEBUG_INFO("csrw(\"%s\", %s " ANSI_MAGENTA "(%p)" ANSI_RESET ")", reg, #val, (void*)x);\
    asm volatile ("csrw " reg ",%0" : : "r" (x));\
} while(0)

// CSR set bits
#define csrs(reg,bits) do {\
    uint64_t x = (uint64_t) bits;\
    DEBUG_INFO("csrs(\"%s\", %s " ANSI_MAGENTA "(%p)" ANSI_RESET ")", reg, #bits, (void*)x);\
    asm volatile ("csrs " reg ",%0" : : "r" (x));\
    DEBUG_INFO("csrs done!");\
} while(0)

// CSR clear bits
#define csrc(reg,bits) do {\
    uint64_t x = (uint64_t) bits;\
    DEBUG_INFO("csrc(\"%s\", %s " ANSI_MAGENTA "(%p)" ANSI_RESET ")", reg, #bits, (void*)x);\
    asm volatile ("csrc " reg ",%0" : : "r" (x));\
} while(0)

// CSR read
#define csrr(reg) ({\
    uint64_t x;\
    asm volatile ("csrr %0," reg : "=r" (x));\
    DEBUG_INFO("csrr(\"%s\") " ANSI_MAGENTA "(%p)" ANSI_RESET, reg, (void*)x);\
    x;\
})

#endif /* CYKOS_CSR_H */
