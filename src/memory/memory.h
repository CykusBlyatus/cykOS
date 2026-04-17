#ifndef CYKOS_MEMORY_H
#define CYKOS_MEMORY_H

#include <stddef.h>

void kheapinit();

void *vmalloc(size_t);
void vfree(void*);

// void *kmalloc(size_t);
// void *kfree(void*);

void *kvmalloc(size_t);
void kvfree(void*);

#endif
