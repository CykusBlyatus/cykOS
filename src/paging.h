#ifndef CYKOS_PAGING_H
#define CYKOS_PAGING_H

#include <stdint.h>
#include <csr.h>

void vminit();

#define PGSHIFT 12            // offset bits per page aka log2(PGSIZE)
#define PGSIZE (1 << PGSHIFT) // page size aka 2^PGSHIFT
#define PGENTRIES (PGSIZE / sizeof(uintptr_t)) // number of entries in a Page Table
#define PGMODE CSR_SATP_SV39

#define PHYSTOP 0x88000000 // what we will consider as the address right after the max valid physical address

// Page Table/Directory
typedef struct {
    uintptr_t entries[PGENTRIES];
} pgtable_t __attribute__((aligned(PGSIZE)));
extern pgtable_t krnl_pgdir; // Kernel root page table, defined in paging.c

#define PTE2PA(pte) (((pte) >> 10) << 12) // Remove flag bits to get physical address
#define PA2PTE(pa) ((((uint64_t)pa) >> 12) << 10) // Clear offset and shift to align with PTE
#define PTE_FLAGS(pte) ((pte) & 0x3FF)

#define PXMASK         0x1FF // 9 bits
#define PXSHIFT(level) (PGSHIFT+(9*(level)))
#define PX(level, va)  ((((uint64_t) (va)) >> PXSHIFT(level)) & PXMASK)

#define PTE_V 1          // Valid bit - if not set, page entry is invalid
#define PTE_R (1 << 1)   // Permission to read
#define PTE_W (1 << 2)   // Permission to write
#define PTE_X (1 << 3)   // Permission to execute
#define PTE_U (1 << 4)   // Permission for user
#define PTE_G (1 << 5)   // Global Bit - if set, the page is accessible regardless of the page table in use
#define PTE_A (1 << 6)   // Accessed Bit - indicates the virtual page has been read, written, or fetched from since the last time the A bit was cleared
#define PTE_D (1 << 7)   // Dirty Bit - indicates the virtual page has been written since the last time the D bit was cleared
#define PTE_RSW (3 << 8) // Unused by hardware, free for the software to use

#define PTE_RW (PTE_R | PTE_W) // Permission to read and write
#define PTE_RX (PTE_R | PTE_X) // Permission to read and execute
#define PTE_RWX (PTE_RW | PTE_X) // Permission to read, write and execute

#if __riscv_xlen == 32
    // Returns the number of paging levels, given the paging mode, or -1 if the mode is unknown
    #define PGLEVELS(mode) ((mode) == CSR_SATP_SV32 ? 1 : (mode) == CSR_SATP_BARE ? 0 : -1)
#elif __riscv_xlen == 64 || __riscv_xlen == 128
    // Returns the number of paging levels, given the paging mode, or -1 if the mode is unknown
    #define PGLEVELS(mode)(\
        (mode) == CSR_SATP_SV39 ? 2\
        : (mode) == CSR_SATP_SV48 ? 3\
        : (mode) == CSR_SATP_SV57 ? 4\
        : (mode) == CSR_SATP_BARE ? 0\
        : -1\
    )
#endif

#endif // CYKOS_PAGING_H
