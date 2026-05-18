#include "paging.h"
#include <proc/lock.h>
#include <include/riscv.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>

// for "panic"
#include <stdio.h>
#include <devices/syscon/syscon.h>

// to get addresses
#include <trap/plic.h>
#include <devices/uart/uart_macros.h>
#include <devices/virtio/blk.h>

#define DEBUG
#include <auxiliary/debug.h>

#define PROTECT_FREE_PAGES // TODO: add compatibility for when this is disabled

#define TLB_FLUSH(va, asid)     asm volatile ("sfence.vma %0, %1" :: "r"(va),"r"(asid) : "memory")
#define TLB_FLUSH_ASID(asid)    asm volatile ("sfence.vma x0, %0" :: "r"(asid) : "memory")
#define TLB_FLUSH_VA_GLOBAL(va) asm volatile ("sfence.vma %0, x0" :: "r"(va) : "memory")
#define TLB_FLUSH_ALL()         asm volatile ("sfence.vma x0, x0" ::: "memory")

DEBUG_PUT(int debug_paging = 0;)

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
    spinlock_t lock;
} kmem; // same structure as pgnode_t but logically different since kmem is not a page

u64 pgalloc() {
    u64 sie = CSRR("sstatus") & CSR_STATUS_SIE;
    CSRC("sstatus", CSR_STATUS_SIE);
    spinlock(&kmem.lock);

    if (!kmem.freelist) {
        DEBUG_WARN("Ran out of memory\n");
        spinrelease(&kmem.lock);
        CSRS("sstatus", sie);
        return 0;
    }
    u64 ret = (u64)kmem.freelist;
    pgnode_t *node = PA2VA(ret);
    volatile pte_t *pte = PA2VA(kmem.pte_head);
    DEBUG_INFO("PTE at VA=%p PA=%p", pte, kmem.pte_head);
    DEBUG_INFO("Node at VA=%p PA=%p", node, kmem.freelist);

    *pte |= PTE_RW | PTE_V;
    kmem.freelist = node->next;
    kmem.pte_head = node->pte_next;
    DEBUG_INFO("next_pa = %p, next_pte_pa = %p", node->next, node->pte_next);
    *pte &= ~(PTE_RWX | PTE_V);

    spinrelease(&kmem.lock);
    CSRS("sstatus", sie);

    DEBUG_INFO("returning");
    return ret;
}

void* pgallocdirect(u8 flags) {
    u64 sie = CSRR("sstatus") & CSR_STATUS_SIE;
    CSRC("sstatus", CSR_STATUS_SIE);
    spinlock(&kmem.lock);

    if (!kmem.freelist) {
        DEBUG_WARN("Ran out of memory\n");
        spinrelease(&kmem.lock);
        CSRS("sstatus", sie);
        return NULL;
    }
    pgnode_t *node = PA2VA(kmem.freelist);
    #ifdef PROTECT_FREE_PAGES
        pte_t *pte_head = PA2VA(kmem.pte_head);
        *pte_head |= flags | PTE_V;
        kmem.pte_head = node->pte_next;
    #endif
    kmem.freelist = node->next;

    spinrelease(&kmem.lock);
    CSRS("sstatus", sie);

    DEBUG_PUT(if (debug_paging) DEBUG_INFO("Allocated page at %p", node);)
    return (void*)node;
}

// int pgmap_pte(pte_t *pte, u64 pa, u8 flags) {
//     *pte = PA2PTE(pa) | flags | PTE_V;
//     return 0;
// }

void pgfree(pgtable_t *root_pgdir, void *va) {
    DEBUG_INFO("Freeing %p", va);
    pgtable_t *pgdir = root_pgdir;
    pte_t *pte;

    va = (void*)(((u64)va) & ~(PGSIZE-1));

    for (int level = PGLEVELS(PGMODE) ;; --level) {
        pte = &pgdir->entries[PX(level, va)];
        // DEBUG_INFO("PTE_VA=%p", pte);
        if (!(*pte & PTE_V))
            panic("Attempted to free unmapped virtual address %p", va);

        if (level == 0)
            break;

        if (*pte & PTE_RWX)
            panic("Superpages not supported\n");
        pgdir = PA2VA(PTE2PA(*pte));
    }

    // get page direct mapping
    u64 pa = PTE2PA(*pte);
    if (va != PA2VA(pa)) { // skip if it already is a direct mapping
        // invalidate page
        *pte &= ~(u64)0x3ff; // clear flags
        TLB_FLUSH(va, 0);

        va = PA2VA(pa);
        // find PTE of direct mapping
        pgdir = &kernel_pgdir;
        for (int level = PGLEVELS(PGMODE) ;; --level) {
            pte = &pgdir->entries[PX(level, va)];
            if (level == 0)
                break;
            pgdir = PTE2VA(*pte);
        }
    }
    *pte |= PTE_RW | PTE_V;

    // add page to freelist
    u64 sie = CSRR("sstatus") & CSR_STATUS_SIE;
    CSRC("sstatus", CSR_STATUS_SIE);
    spinlock(&kmem.lock);

    pgnode_t *node = va;
    node->next = kmem.freelist;
    node->pte_next = kmem.pte_head;

    kmem.freelist = (void*)pa;
    kmem.pte_head = (void*)VA2PA(pte);

    spinrelease(&kmem.lock);
    CSRS("sstatus", sie);

    // invalidate direct mapping
    *pte &= ~(PTE_RW | PTE_V);
    TLB_FLUSH(va, 0);

    DEBUG_INFO("returning");
}

pte_t* pgmap(pgtable_t *pgdir, u64 pa, void *va, u8 flags) {
    if (pgdir != &kernel_pgdir)
        panic("Multiple page tables not supported yet");

    if (((u64)va & (PGSIZE-1)) != 0)
        panic("va (%p) %% PGSIZE (0x%x) != 0\n", va, PGSIZE);

    DEBUG_PUT(if (debug_paging) DEBUG_INFO("Mapping %p to %p...", va, (void*)pa);)

    for (int level = PGLEVELS(PGMODE); level > 0; --level) {
        pte_t *pte = &pgdir->entries[PX(level, va)];
        DEBUG_PUT(if (debug_paging) DEBUG_INFO("PTE at %p", pte));
        if(*pte & PTE_V) {
            pgdir = PTE2VA(*pte);
        } else {
            panic("Tried to call pgmap on unmapped virtual address %p", va);
        }
        DEBUG_PUT(if (debug_paging) DEBUG_INFO("PTE = %p, pgdir = %p", (void*)*pte, pgdir);)
    }

    pte_t *pte = &pgdir->entries[PX(0, va)];
    *pte = PA2PTE(pa) | flags | PTE_V;
    TLB_FLUSH(va, 0);
    return pte;
}

// pgmap in physical mode
static pte_t* physpgmap(pgtable_t *pgdir, u64 pa, void *va, u8 flags) {
    if (pgdir != &kernel_pgdir)
        panic("Multiple page tables not supported yet");

    if (((u64)va & (PGSIZE-1)) != 0)
        panic("va (%p) %% PGSIZE (0x%x) != 0\n", va, PGSIZE);

    DEBUG_PUT(if (debug_paging) DEBUG_INFO("Mapping %p to %p...", va, (void*)pa);)

    for (int level = PGLEVELS(PGMODE); level > 0; --level) {
        pte_t *pte = &pgdir->entries[PX(level, va)];
        if(*pte & PTE_V) {
            pgdir = (void*)PTE2PA(*pte);
        } else {
            if ((pgdir = (pgtable_t*)kmem.freelist) == NULL) {
                DEBUG_WARN("Ran out of memory");
                return NULL;
            }
            kmem.pte_head = kmem.freelist->pte_next;
            kmem.freelist = kmem.freelist->next;
            // __builtin_memcpy(&kmem, kmem.freelist, sizeof(pgnode_t));

            __builtin_memset(__builtin_assume_aligned(pgdir, PGSIZE), 0, PGSIZE);
            *pte = PA2PTE(pgdir) | PTE_V;
            physpgmap(&kernel_pgdir, (u64)pgdir, PA2VA(pgdir), PTE_RW);
        }
        DEBUG_PUT(if (debug_paging) DEBUG_INFO("PTE = %p, pgdir = %p", (void*)*pte, pgdir);)
    }

    pte_t *pte = &pgdir->entries[PX(0, va)];
    *pte = PA2PTE(pa) | flags | PTE_V;
    DEBUG_PUT(if (debug_paging) DEBUG_INFO("VA=%p PA=%p PTE2PA=%p", va, (void*)pa, (void*)PTE2PA(*pte)));
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
        physpgmap(&kernel_pgdir, (u64)addr, addr, PTE_RX);

    DEBUG_INFO("Mapping kernel rodata (%p-%p)...", kernel_rodata_start, kernel_rodata_end);
    for (void *addr = kernel_rodata_start; addr < (void*)kernel_rodata_end; addr += PGSIZE)
        physpgmap(&kernel_pgdir, (u64)addr, addr, PTE_R);

    DEBUG_INFO("Mapping kernel data and bss (%p-%p)...", kernel_data_start, kernel_bss_end);
    for (void *addr = kernel_data_start; addr < (void*)kernel_bss_end; addr += PGSIZE)
        physpgmap(&kernel_pgdir, (u64)addr, addr, PTE_RW);

    DEBUG_INFO("Mapping UART (%p-%p)...", (void*)UART0_BASE, (void*)UART0_BASE + PGSIZE);
    physpgmap(&kernel_pgdir, (u64)UART0_BASE, (void*)UART0_BASE, PTE_RW);

    // PLIC is so big that printing debug for it slows down this function
    DEBUG_PUT(int debug_paging_ = debug_paging;);
    DEBUG_PUT(debug_paging = 0;)
    DEBUG_INFO("Mapping PLIC (%p-%p)...", (void*)PLIC_BASE, (void*)PLIC_BASE + 0x4000000);
    for (void *addr = (void*)PLIC_BASE; addr < (void*)PLIC_BASE + 0x4000000; addr += PGSIZE)
        physpgmap(&kernel_pgdir, (u64)addr, addr, PTE_RW);
    DEBUG_PUT(debug_paging = debug_paging_;)

    DEBUG_INFO("Mapping SYSCON (%p-%p)...", (void*)SYSCON_ADDR, (void*)SYSCON_ADDR + PGSIZE);
    physpgmap(&kernel_pgdir, (u64)SYSCON_ADDR, (void*)SYSCON_ADDR, PTE_RW);

    // map clint as read-only just so I can read mtime
    void *clint = (void*)0x2000000;
    DEBUG_INFO("Mapping CLINT (%p-%p)...", clint, clint + 0x10000);
    for (void *addr = clint; addr < clint + 0x10000; addr += PGSIZE)
        physpgmap(&kernel_pgdir, (u64)addr, addr, PTE_R);

    physpgmap(&kernel_pgdir, (u64)&DISK0(0), (void*)&DISK0(0), PTE_RW);

    for (void *addr = (void*)KERNEL_HEAP_START; addr != (void*)KERNEL_HEAP_END; addr += PGSIZE)
        *physpgmap(&kernel_pgdir, 0, addr, 0) &= ~PTE_V;

    for (pgnode_t *node = (pgnode_t*)&kmem; node->next != NULL; node = node->next) {
        #ifdef PROTECT_FREE_PAGES
            node->pte_next = physpgmap(&kernel_pgdir, (u64)node->next, PA2VA(node->next), 0);
            *node->pte_next &= ~PTE_V;
        #else
            physpgmap(&kernel_pgdir, (u64)node->next, PA2VA(node->next), PTE_RW);
        #endif
    }

    DEBUG_INFO("Kernel root page table at %p", &kernel_pgdir);
    CSRW("satp", PGMODE | ((uptr)&kernel_pgdir >> PGSHIFT));
    TLB_FLUSH_ALL();
}
