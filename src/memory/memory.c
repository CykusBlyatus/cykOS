#include "memory.h"
#include "paging.h"
#include <stdio.h>
#include "auxiliary/debug.h"
#include <proc/lock.h>

#define ALIGNMENT 16

#define PANIC_HEAP_CORRUPTION(node) panic("Kernel Heap Corruption (node at %p has node->next=%p, but node->size=%p (+sizeof(*node)=%p))", node, (node)->next, (void*)(node)->size, (void*)((node)->size + sizeof(*(node))))

typedef struct kheapnode {
    size_t size;
    struct kheapnode *next;
    struct kheapnode *previous;
    u8 free;
} __attribute__((aligned(ALIGNMENT))) kheapnode_t;

static lock_t lock;
static kheapnode_t *head;

void kheapinit() {
    head = (kheapnode_t*)KERNEL_HEAP_START;
    *head = (kheapnode_t) {
        .size = KERNEL_HEAP_SIZE - sizeof(*head),
        .free = 1,
        .next = NULL,
        .previous = NULL,
    };
}

void* vmalloc(size_t size) {
    sleeplock(&lock);

    // if (size % ALIGNMENT) size += ALIGNMENT - (size % ALIGNMENT);
    size = (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);

    for (kheapnode_t *node = head; node != NULL; node = node->next) {

        if (!node->free || size > node->size) {
            if (node->next && node->next != (void*)(node+1) + node->size)
                PANIC_HEAP_CORRUPTION(node);
            continue;
        }

        node->free = 0;
        if (size >= node->size - sizeof(*node) - ALIGNMENT) {
            release(&lock);
            return node+1;
        }

        kheapnode_t *newnode = ((void*)(node+1)) + size;
        *newnode = (kheapnode_t) {
            .size = node->size - size - sizeof(*node),
            .next = node->next,
            .previous = node,
            .free = 1,
        };

        if (node->next)
            node->next->previous = newnode;

        node->size = size;
        node->next = newnode;
        release(&lock);
        return node+1;
    }

    release(&lock);
    DEBUG_WARN("Kernel Heap ran out of memory");
    return NULL;
}

void vfree(void *p) {
    if (!p) return;

    sleeplock(&lock);

    kheapnode_t *node = ((kheapnode_t*)p)-1;
    node->free = 1;

    kheapnode_t *next = node->next;
    if (next) {
        if (next != (void*)(node+1) + node->size)
            PANIC_HEAP_CORRUPTION(node);

        if (next->free) {
            node->size += next->size + sizeof(*next);
            node->next = next->next;
            if (next->next)
                next->next->previous = node;
        }
    }

    kheapnode_t *previous = node->previous;
    if (previous) {
        if (node != (void*)(previous+1) + previous->size)
            // this panic is less likely to be triggered than others, since node->previous could be corrupted and trigger a page fault
            PANIC_HEAP_CORRUPTION(previous);

        if (previous->free) {
            previous->size += node->size + sizeof(*node);
            previous->next = node->next;
            if (node->next)
                node->next->previous = previous;
        }
    }

    release(&lock);
}

void *kvmalloc(size_t size) {
    return vmalloc(size);
}

void kvfree(void *p) {
    vfree(p);
}
