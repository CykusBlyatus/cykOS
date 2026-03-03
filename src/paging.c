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
#include <virtio/blk.h>

#define DEBUG
#include <auxiliary/debug.h>

#define PROTECT_FREE_PAGES

#define TLB_FLUSH_VA(va, asid)  asm volatile ("sfence.vma %0, %1" :: "r"(va),"r"(asid) : "memory")
#define TLB_FLUSH_AS(asid)      asm volatile ("sfence.vma x0, %0" :: "r"(asid) : "memory")
#define TLB_FLUSH_VA_GLOBAL(va) asm volatile ("sfence.vma %0, x0" :: "r"(va) : "memory")
#define TLB_FLUSH_ALL()         asm volatile ("sfence.vma x0, x0" ::: "memory")

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

typedef struct pgtable {
    pte_t entries[PGENTRIES];
} pgtable_t __attribute__((aligned(PGSIZE)));
pgtable_t kernel_pgdir;

// Node from a linked list of free pages
typedef struct pgnode {
    struct pgnode *next;
    #ifdef PROTECT_FREE_PAGES
        pte_t *pte_next;
    #endif
} pgnode_t;

static struct {
    pgnode_t *freelist; // Linked list of free pages
    #ifdef PROTECT_FREE_PAGES
        pte_t *pte_head;
    #endif
} kmem; // same structure as pgnode_t but logically different since kmem is not a page

void *pgalloc() {
    pgnode_t *node = kmem.freelist;
    if (!node) {
        DEBUG_ERROR("Ran out of memory\n");
        return NULL;
    }
    #ifdef PROTECT_FREE_PAGES
        *kmem.pte_head |= PTE_RW | PTE_V;
        kmem.pte_head = node->pte_next;
    #endif
    kmem.freelist = node->next;
    DEBUG_PUT(if (debug_paging) DEBUG_INFO("Allocated page at %p", node);)
    return node;
}

void pgfree(pgtable_t *root_pgdir, void *va) {
    pgtable_t *pgdir = root_pgdir;
    pte_t *pte;
    for (int level = PGLEVELS(PGMODE) ;; --level) {
        pte = &pgdir->entries[PX(level, va)];
        if (!(*pte & PTE_V)) {
            DEBUG_ERROR("Attempted to free unmapped virtual address %p", va);
            poweroff();
        }

        if (level == 0)
            break;

        if (*pte & PTE_RWX) {
            DEBUG_ERROR("Superpages not supported\n");
            poweroff();
        }
        pgdir = (pgtable_t*)PTE2PA(*pte);
    }

    if (root_pgdir == &kernel_pgdir) {
        #ifdef PROTECT_FREE_PAGES
            if (!(*pte & PTE_RWX)) {
                DEBUG_ERROR("Attempted page double-free at virtual address %p", va);
                poweroff();
            }
            *pte &= ~PTE_RWX;
        #else
            *pte &= ~PTE_X;
            *pte |= PTE_RW;
        #endif
    } else {
        *pte = 0;
        DEBUG_ERROR("Multiple page tables not supported yet");
        poweroff();
    }

    kmem.freelist = (pgnode_t*)PTE2PA(*pte);
    #ifdef PROTECT_FREE_PAGES
        kmem.pte_head = pte;
    #endif

    TLB_FLUSH_VA(va, 0);
}

pte_t* pgmap(pgtable_t *pgdir, void *pa, void *va, u16 flags) {
    if (pgdir != &kernel_pgdir) {
        DEBUG_ERROR("Multiple page tables not supported yet");
        poweroff();
    }

    if (((uptr)va & (PGSIZE-1)) != 0) {
        DEBUG_ERROR("va (%p) %% PGSIZE (%x) != 0\n", va, PGSIZE);
        poweroff(); // die
    }

    DEBUG_PUT(if (debug_paging) DEBUG_INFO("Mapping %p to %p...", va, pa);)

    for (int level = PGLEVELS(PGMODE); level > 0; --level) {
        pte_t *pte = &pgdir->entries[PX(level, va)];
        if(*pte & PTE_V) {
            pgdir = (void*)PTE2PA(*pte);
        } else {
            if ((pgdir = (pgtable_t*)kmem.freelist) == NULL) {
                DEBUG_ERROR("Ran out of memory");
                return NULL;
            }
            *(pgnode_t*)&kmem = *(pgnode_t*)pgdir;
            __builtin_memset(__builtin_assume_aligned(pgdir, PGSIZE), 0, PGSIZE);
            *pte = PA2PTE(pgdir) | PTE_V;
            #ifdef PROTECT_FREE_PAGES
                pgmap(&kernel_pgdir, pgdir, pgdir, PTE_RW);
            #endif
        }
        DEBUG_PUT(if (debug_paging) DEBUG_INFO("PTE = %p, pgdir = %p", (void*)*pte, pgdir);)
    }

    pte_t *pte = &pgdir->entries[PX(0, va)];
    *pte = PA2PTE(pa) | flags | PTE_V;
    return pte;
}

void pginit() {
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

    #ifdef PROTECT_FREE_PAGES
    {
        for (pgnode_t *node = (pgnode_t*)&kmem; node->next != NULL; node = node->next) {
            node->pte_next = pgmap(&kernel_pgdir, node->next, node->next, 0);
            *node->pte_next &= ~PTE_V;
        }
    }
    #else
    {
        for (void *addr = kernel_end; addr < (void*)PHYSTOP; addr += PGSIZE)
            pgmap(&kernel_pgdir, addr, addr, PTE_RW);
    }
    #endif

    DEBUG_INFO("Kernel root page table at %p", &kernel_pgdir);
    CSRW("satp", PGMODE | ((uptr)&kernel_pgdir >> PGSHIFT));
}
