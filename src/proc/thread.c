#define DEBUG
#include <auxiliary/debug.h>

#include "thread.h"
#include <memory/memory.h>
#include <memory/paging.h>
#include <stddef.h>
#include <include/riscv.h>
#include <stdio.h>
#include <string.h>
#include <devices/timer/timer.h>
#include "lock.h"

typedef enum {
    SLEEPING,
    READY,
    RUNNING,
} threadstate_t;

typedef struct kthread {
    cpucontext_t ctx;
    // u32 id;
    struct kthread *next, *prev;
    void *chan;
    threadstate_t state;
    // spinlock_t lock;
} kthread_t;

static kthread_t init_thread;
kthread_t *current_thread = &init_thread;

static spinlock_t lock; // global lock for now

void yield() {
    asm volatile ("ecall");
}

void sleepchan(void *chan, spinlock_t *lk) {
    spinlock(&lock);

    current_thread->state = SLEEPING;
    current_thread->chan = chan;

    spinrelease(&lock);
    if (lk)
        spinrelease(lk);

    CSRS("sstatus", CSR_STATUS_SIE);
    yield();

    if (lk)
        spinlock(lk);
}

void wakeupchan(void *chan) {
    u64 sie = CSRR("sstatus") & CSR_STATUS_SIE;
    CSRC("sstatus", CSR_STATUS_SIE);
    spinlock(&lock);

    kthread_t *thrd = current_thread->next;
    do {
        DEBUG_INFO("%p, %p", thrd->chan, chan);
        if (thrd->state != SLEEPING || thrd->chan != chan)
            continue;

        DEBUG_SUCCESS("woke up a thread!");
        thrd->state = READY;
        thrd->chan = NULL;
        thrd = thrd->next;
    } while (thrd != current_thread);

    spinrelease(&lock);
    CSRS("sstatus", sie);
}

void sched(cpucontext_t *current_ctx) {
    while (1) {
        u64 sie = CSRR("sstatus") & CSR_STATUS_SIE;
        CSRC("sstatus", CSR_STATUS_SIE);
        spinlock(&lock);

        kthread_t *thrd = current_thread->next;
        do {
            DEBUG_INFO("%d", thrd->state);
            if (thrd->state != READY) {
                thrd = thrd->next;
                continue;
            }

            if (current_ctx) {
                current_thread->ctx = *current_ctx;
                *current_ctx = thrd->ctx;
            }
            if (current_thread->state == RUNNING)
                current_thread->state = READY;
            current_thread = thrd;
            current_thread->state = RUNNING;
            break;
        } while (thrd != current_thread);

        spinrelease(&lock);
        CSRS("sstatus", sie);

        if (current_thread->state == RUNNING)
            break;

        if (sie & CSR_STATUS_SIE) {
            DEBUG_INFO("No threads to run, guess I'll sleep");
            u64 sstatus = CSRR("sstatus");
            asm volatile ("wfi");
            CSRW("sstatus", sstatus);
        } else {
            panic("No threads to run and interrupts are disabled");
            // DEBUG_INFO("No threads to run and interrupts disabled, guess I'll try to schedule a thread again");
        }
    }

    DEBUG_INFO("new current_thread = %p", current_thread);

    DEBUG_INFO("Registers after context switch:");
    for (int i = 0; i < 31; ++i)
        DEBUG_INFO("x%d (%s) = %p (%llu)", i+1, regnames[i+1], (void*)current_thread->ctx.regs[i], current_thread->ctx.regs[i]);
}

void kthread_init() {
    *current_thread = (kthread_t) {
        .state = RUNNING,
        .next = current_thread,
        .prev = current_thread,
    };
    CSRW("stimecmp", mtime + SCHED_QUANTUM);
    CSRS("sie", CSR_IEIP_STI);
}

__attribute__((noreturn, optimize("omit-frame-pointer"))) static void kthread_end() {
    // this implementation is awkward, will probably be replaced in the future
    DEBUG_INFO("called");
    register kthread_t *thrd = current_thread;

    CSRC("sstatus", CSR_STATUS_SIE);
    sched(NULL);

    spinlock(&lock);
    thrd->prev->next = thrd->next;
    thrd->next->prev = thrd->prev;
    spinrelease(&lock);

    // save stack pointer of thread we are ending
    register void *dangling_stack;
    asm volatile("mv %0,sp" : "=r"(dangling_stack));

    // load stack from other thread and add a frame for ctx
    asm volatile(
        "mv sp,%0\n"
        "addi sp,sp,-%1\n"
        :: "r"(current_thread->ctx.sp), "i"(sizeof(cpucontext_t))
    );

    // copy ctx to stack
    register cpucontext_t *sp asm("sp");
    *sp = current_thread->ctx;

    // make kvfree and pgfree preemptible
    CSRS("sstatus", CSR_STATUS_SIE);

    kvfree(thrd);

    pgfree(&kernel_pgdir, dangling_stack);

    // disable preemption and set things up to continue other thread
    CSRC("sstatus", CSR_STATUS_SIE);
    CSRS("sstatus", CSR_STATUS_SPP);
    CSRW("sepc", sp->pc);

    asm volatile(
        // Restore context
        "ld ra,0(sp)\n"
        // "ld sp,8(sp)\n"
        "ld gp,16(sp)\n"
        "ld tp,24(sp)\n"
        "ld t0,32(sp)\n"
        "ld t1,40(sp)\n"
        "ld t2,48(sp)\n"
        "ld s0,56(sp)\n"
        "ld s1,64(sp)\n"
        "ld a0,72(sp)\n"
        "ld a1,80(sp)\n"
        "ld a2,88(sp)\n"
        "ld a3,96(sp)\n"
        "ld a4,104(sp)\n"
        "ld a5,112(sp)\n"
        "ld a6,120(sp)\n"
        "ld a7,128(sp)\n"
        "ld s2,136(sp)\n"
        "ld s3,144(sp)\n"
        "ld s4,152(sp)\n"
        "ld s5,160(sp)\n"
        "ld s6,168(sp)\n"
        "ld s7,176(sp)\n"
        "ld s8,184(sp)\n"
        "ld s9,192(sp)\n"
        "ld s10,200(sp)\n"
        "ld s11,208(sp)\n"
        "ld t3,216(sp)\n"
        "ld t4,224(sp)\n"
        "ld t5,232(sp)\n"
        "ld t6,240(sp)\n"

        "addi sp,sp,%0\n"

        "sret\n"

        :: "i"(sizeof(cpucontext_t))
        : "memory"
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

    u64 sie = CSRR("sstatus") & CSR_STATUS_SIE;
    CSRC("sstatus", CSR_STATUS_SIE);
    spinlock(&lock);

    current_thread->next = newthread;
    if (current_thread->prev == current_thread)
        current_thread->prev = newthread;

    spinrelease(&lock);
    CSRS("sstatus", sie);
    return 0;
}
