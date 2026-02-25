#include "paging.h"
#include "csr.h"
#include <stdint.h>
#include <string.h>
#include <stddef.h>

// for "panic"
#include <stdio.h>
#include "syscon.h"

// to get addresses
#include "plic.h"
#include "uart_macros.h"

#define DEBUG
#include <auxiliary/debug.h>

DEBUG_PUT(int debug_paging = 0;)
const size_t pgsize = PGSIZE; // for linker.ld

// Defined in linker.ld
extern char
    kernel_start[],
    kernel_text_start[],
    kernel_text_end[],
    kernel_rodata_start[],
    kernel_rodata_end[],
    kernel_data_start[],
    kernel_data_end[],
    kernel_bss_start[],
    kernel_bss_end[],
    kernel_end[];

pgtable_t kernel_pgdir;

// Node from a linked list of pages
typedef struct pgnode {
    struct pgnode *next;
} pgnode_t;

static struct {
    pgnode_t *freelist; // Linked list of free pages
} kmem;

// Basically a copy of https://github.com/mit-pdos/xv6-riscv/blob/de247db5e6384b138f270e0a7c745989b5a9c23b/kernel/kalloc.c#L69
void *pgalloc() {
    pgnode_t *node = kmem.freelist;
    if (node)
        kmem.freelist = node->next;
    DEBUG_PUT(if (debug_paging) DEBUG_INFO("Allocated page at %p", node);)
    return node;
}

// Basically a copy of https://github.com/mit-pdos/xv6-riscv/blob/de247db5e6384b138f270e0a7c745989b5a9c23b/kernel/vm.c#L144
int pgmap(pgtable_t *pgdir, void *pa, void *va, int perm) {
    if ((uintptr_t)va % PGSIZE != 0) {
        DEBUG_ERROR("va (%p) %% PGSIZE (%x) != 0\n", va, PGSIZE);
        poweroff(); // die
    }

    DEBUG_PUT(if (debug_paging) DEBUG_INFO("Mapping %p to %p...", va, pa);)

    for (int level = PGLEVELS(PGMODE); level > 0; --level) {
        uintptr_t *pte = &pgdir->entries[PX(level, va)];
        if(*pte & PTE_V) {
            pgdir = (void*)PTE2PA(*pte);
        } else {
            if ((pgdir = pgalloc()) == NULL)
                return -1;
            memset(pgdir, 0, PGSIZE);
            *pte = PA2PTE(pgdir) | PTE_V;
        }
        DEBUG_PUT(if (debug_paging) DEBUG_INFO("PTE = %p, pgdir = %p", (void*)*pte, pgdir);)
    }

    uintptr_t *pte = &pgdir->entries[PX(0, va)];
    *pte = PA2PTE(pa) | perm | PTE_V;
    return 0;
}

void vminit() {
    DEBUG_INFO("called");

    DEBUG_INFO("Marking all pages outisde kernel as free...");
    for (void *addr = kernel_end; addr < (void*)PHYSTOP; addr += PGSIZE) {
        pgnode_t *node = addr;
        node->next = kmem.freelist;
        kmem.freelist = node;
    }

    DEBUG_INFO("Mapping kernel text (%p-%p)...", kernel_text_start, kernel_text_end);
    for (void *addr = kernel_text_start; addr < (void*)kernel_text_end; addr += PGSIZE)
        pgmap(&kernel_pgdir, addr, addr, PTE_RX);

    DEBUG_INFO("Mapping kernel rodata (%p-%p)...", kernel_rodata_start, kernel_rodata_end);
    for (void *addr = kernel_rodata_start; addr < (void*)kernel_rodata_end; addr += PGSIZE)
        pgmap(&kernel_pgdir, addr, addr, PTE_R);

    DEBUG_INFO("Mapping kernel data and bss (%p-%p)...", kernel_data_start, kernel_bss_end);
    for (void *addr = kernel_data_start; addr < (void*)kernel_bss_end; addr += PGSIZE)
        pgmap(&kernel_pgdir, addr, addr, PTE_RW);

    DEBUG_INFO("Mapping UART (%p-%p)...", (void*)UART0_BASE, (void*)UART0_BASE + PGSIZE);
    pgmap(&kernel_pgdir, (void*)UART0_BASE, (void*)UART0_BASE, PTE_RW);

    // PLIC is so big that printing debug for it slows down this function
    DEBUG_PUT(int debug_paging_ = debug_paging;);
    DEBUG_PUT(debug_paging = 0;)
    DEBUG_INFO("Mapping PLIC (%p-%p)...", (void*)PLIC_BASE, (void*)PLIC_BASE + 0x4000000);
    for (void *addr = (void*)PLIC_BASE; addr < (void*)PLIC_BASE + 0x4000000; addr += PGSIZE)
        pgmap(&kernel_pgdir, addr, addr, PTE_RW);
    DEBUG_PUT(debug_paging = debug_paging_;)

    DEBUG_INFO("Mapping SYSCON (%p-%p)...", (void*)SYSCON_ADDR, (void*)SYSCON_ADDR + PGSIZE);
    pgmap(&kernel_pgdir, (void*)SYSCON_ADDR, (void*)SYSCON_ADDR, PTE_RW);

    // map clint as read-only just so I can read mtime
    void *clint = (void*)0x2000000;
    DEBUG_INFO("Mapping CLINT (%p-%p)...", clint, clint + 0x10000);
    for (void *addr = clint; addr < clint + 0x10000; addr += PGSIZE)
        pgmap(&kernel_pgdir, addr, addr, PTE_R);

    DEBUG_INFO("Kernel root page table at %p", &kernel_pgdir);
    CSRW("satp", PGMODE | ((uintptr_t)&kernel_pgdir >> PGSHIFT));
}
