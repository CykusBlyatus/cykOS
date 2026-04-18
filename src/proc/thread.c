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

void sleepchan(void *chan) {
    u64 sie = CSRR("sstatus") & CSR_STATUS_SIE;
    CSRC("sstatus", CSR_STATUS_SIE);
    spinlock(&lock);

    current_thread->state = SLEEPING;
    current_thread->chan = chan;

    spinrelease(&lock);
    CSRS("sstatus", sie);

    yield();
}

void wakeupchan(void *chan) {
    u64 sie = CSRR("sstatus") & CSR_STATUS_SIE;
    CSRC("sstatus", CSR_STATUS_SIE);
    spinlock(&lock);

    for (kthread_t *thrd = current_thread->next; thrd != current_thread; thrd = thrd->next) {
        if (thrd->state != SLEEPING || thrd->chan != chan)
            continue;

        thrd->state = READY;
        thrd->chan = NULL;
    }

    spinrelease(&lock);
    CSRS("sstatus", sie);
}

void sched(cpucontext_t *current_ctx) {
    while (1) {
        u64 sie = CSRR("sstatus") & CSR_STATUS_SIE;
        CSRC("sstatus", CSR_STATUS_SIE);
        spinlock(&lock);

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
            if (current_thread->state == RUNNING)
                current_thread->state = READY;
            current_thread = thrd;
            current_thread->state = RUNNING;
            break;
        }

        spinrelease(&lock);
        CSRS("sstatus", sie);

        if (current_thread->state == RUNNING)
            break;

        if (sie & CSR_STATUS_SIE) {
            DEBUG_INFO("No threads to run, guess I'll sleep");
            asm volatile ("wfi");
        } else {
            DEBUG_INFO("No threads to run and interrupts disabled, guess I'll try to schedule a thread again");
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

const char str[] = "meowl\n";

__attribute__((noreturn)) static void kthread_end() {
    DEBUG_INFO("called");
    kthread_t *thrd = current_thread;

    CSRC("sstatus", CSR_STATUS_SIE);
    sched(NULL);

    spinlock(&lock);
    thrd->prev->next = thrd->next;
    thrd->next->prev = thrd->prev;
    spinrelease(&lock);

    asm volatile(
        "mv s0,%[thrd]\n" // store old thread pointer for kvfree
        "ld s3,current_thread\n"
        "mv s1,sp\n" // save dead thread's stack pointer
        "li s2,2\n" // CSR_STATUS_SIE

        // load other thread's stack, put ctx in it
        "ld sp,8(s3)\n"
        "addi sp,sp,-256\n"
        "ld t0,0(s3)\nsd t0,0(sp)\n"
        // "ld t0,8(s3)\nsd t0,8(sp)\n"
        "ld t0,16(s3)\nsd t0,16(sp)\n"
        "ld t0,24(s3)\nsd t0,24(sp)\n"
        "ld t0,32(s3)\nsd t0,32(sp)\n"
        "ld t0,40(s3)\nsd t0,40(sp)\n"
        "ld t0,48(s3)\nsd t0,48(sp)\n"
        "ld t0,56(s3)\nsd t0,56(sp)\n"
        "ld t0,64(s3)\nsd t0,64(sp)\n"
        "ld t0,72(s3)\nsd t0,72(sp)\n"
        "ld t0,80(s3)\nsd t0,80(sp)\n"
        "ld t0,88(s3)\nsd t0,88(sp)\n"
        "ld t0,96(s3)\nsd t0,96(sp)\n"
        "ld t0,104(s3)\nsd t0,104(sp)\n"
        "ld t0,112(s3)\nsd t0,112(sp)\n"
        "ld t0,120(s3)\nsd t0,120(sp)\n"
        "ld t0,128(s3)\nsd t0,128(sp)\n"
        "ld t0,136(s3)\nsd t0,136(sp)\n"
        "ld t0,144(s3)\nsd t0,144(sp)\n"
        "ld t0,152(s3)\nsd t0,152(sp)\n"
        "ld t0,160(s3)\nsd t0,160(sp)\n"
        "ld t0,168(s3)\nsd t0,168(sp)\n"
        "ld t0,176(s3)\nsd t0,176(sp)\n"
        "ld t0,184(s3)\nsd t0,184(sp)\n"
        "ld t0,192(s3)\nsd t0,192(sp)\n"
        "ld t0,200(s3)\nsd t0,200(sp)\n"
        "ld t0,208(s3)\nsd t0,208(sp)\n"
        "ld t0,216(s3)\nsd t0,216(sp)\n"
        "ld t0,224(s3)\nsd t0,224(sp)\n"
        "ld t0,232(s3)\nsd t0,232(sp)\n"
        "ld t0,240(s3)\nsd t0,240(sp)\n"
        "ld t0,248(s3)\nsd t0,248(sp)\n"

        // enable interrupts
        "csrs sstatus,s2\n"

        // pgfree(&kernel_pgdir, dead_thread_stack_ptr)
        "la a0,kernel_pgdir\n"
        "mv a1,s1\n"
        "call pgfree\n"

        // kvfree(thrd)
        "mv a0,s0\n"
        "call kvfree\n"

        // disable interrupts
        "csrc sstatus,s2\n"

        // Set CSR_STATUS_SPP
        "li t0,256\n"
        "csrs sstatus,t0\n"

        // Set sepc
        "ld t0,248(sp)\n"
        "csrw sepc,t0\n"

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

        "addi sp,sp,256\n"

        "sret\n"

        :: [thrd]"r"(thrd)
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
