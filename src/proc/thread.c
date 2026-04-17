#define DEBUG

#include "thread.h"
#include "auxiliary/debug.h"
#include <memory/memory.h>
#include <memory/paging.h>
#include <stddef.h>
#include <include/riscv.h>
#include <stdio.h>
#include <string.h>

typedef enum {
    SLEEPING,
    READY,
    RUNNING,
} threadstate_t;

typedef struct kthread {
    cpucontext_t ctx;
    // u32 id;
    threadstate_t state;
    // lock_t lock;
    struct kthread *next, *prev;
    void *chan;
} kthread_t;

static kthread_t init_thread;
kthread_t *current_thread = &init_thread;

void yield() {
    asm volatile ("ecall");
}

int sleep(void *chan) {
    current_thread->state = SLEEPING;
    current_thread->chan = chan;
    yield();
    return 0;
}

int wakeup(void *chan) {
    for (kthread_t *thrd = current_thread->next; thrd != current_thread; thrd = thrd->next) {
        if (thrd->state != SLEEPING || thrd->chan != chan)
            continue;

        thrd->state = READY;
        thrd->chan = NULL;
    }
    return 0;
}

int sched(cpucontext_t *current_ctx) {
    if (current_thread->state == RUNNING)
        current_thread->state = READY;

    for (
        kthread_t *thrd = current_thread->next;
        thrd != current_thread;
        thrd = thrd->next
    ) {
        if (thrd->state != READY)
            continue;

        if (current_ctx) {
            current_thread->ctx = *current_ctx;
            *current_ctx = thrd->ctx;
        }
        current_thread = thrd;
        break;
    }

    current_thread->state = RUNNING;
    DEBUG_INFO("new current_thread = %p", current_thread);

    DEBUG_INFO("Registers after context switch:");
    for (int i = 0; i < 31; ++i)
        DEBUG_INFO("x%d (%s) = %p (%llu)", i+1, regnames[i+1], (void*)current_thread->ctx.regs[i], current_thread->ctx.regs[i]);
    return 0;
}

void kthread_init() {
    *current_thread = (kthread_t) {
        .state = RUNNING,
        .next = current_thread,
        .prev = current_thread,
    };
}

__attribute__((noreturn)) static void kthread_end() {
    DEBUG_INFO("called");
    kthread_t *thrd = current_thread;

    sched(NULL);

    thrd->prev->next = thrd->next;
    thrd->next->prev = thrd->prev;

    kvfree(thrd);
    CSRW("sepc", current_thread->ctx.pc);
    CSRS("sstatus", CSR_STATUS_SPP);

    asm volatile(
        // store dead thread's stack pointer for pgfree
        "mv a1,sp\n"

        // load stack pointer from other thread
        "ld s0,current_thread\n"
        "ld sp,8(s0)\n"

        // free dead thread's stack
        "la a0,kernel_pgdir\n"
        "call pgfree\n"

        // load other thread's context
        "ld ra,0(s0)\n"
        // "ld sp,8(s0)\n"
        "ld gp,16(s0)\n"
        "ld tp,24(s0)\n"
        "ld t0,32(s0)\n"
        "ld t1,40(s0)\n"
        "ld t2,48(s0)\n"
        // "ld s0,56(s0)\n"
        "ld s1,64(s0)\n"
        "ld a0,72(s0)\n"
        "ld a1,80(s0)\n"
        "ld a2,88(s0)\n"
        "ld a3,96(s0)\n"
        "ld a4,104(s0)\n"
        "ld a5,112(s0)\n"
        "ld a6,120(s0)\n"
        "ld a7,128(s0)\n"
        "ld s2,136(s0)\n"
        "ld s3,144(s0)\n"
        "ld s4,152(s0)\n"
        "ld s5,160(s0)\n"
        "ld s6,168(s0)\n"
        "ld s7,176(s0)\n"
        "ld s8,184(s0)\n"
        "ld s9,192(s0)\n"
        "ld s10,200(s0)\n"
        "ld s11,208(s0)\n"
        "ld t3,216(s0)\n"
        "ld t4,224(s0)\n"
        "ld t5,232(s0)\n"
        "ld t6,240(s0)\n"

        "ld s0,56(s0)\n"

        "sret\n"
    );
    while (1);
}

int kthread_start(void (*func)(void*), void *data) {
    void *sp = pgallocdirect(PTE_RW);
    if (!sp) return -1;
    sp += PGSIZE;

    kthread_t *newthread = kvmalloc(sizeof(*newthread));
    if (!newthread) return -1;

    *newthread = (kthread_t) {
        .ctx = {
            .ra = (cpureg_t)kthread_end,
            .sp = (cpureg_t)sp,
            .a0 = (cpureg_t)data,
            .pc = func,
        },
        .state = READY,
        .prev = current_thread,
        .next = current_thread->next,
    };

    current_thread->next = newthread;
    if (current_thread->prev == current_thread)
        current_thread->prev = newthread;
    // yield();
    return 0;
}
