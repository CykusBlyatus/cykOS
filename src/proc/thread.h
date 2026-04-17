#ifndef CYKOS_THREAD_H
#define CYKOS_THREAD_H

#include <riscv.h>

// Puts the thread to sleep on the channel until another thread wakes it up
int sleep(void *chan);

// Wakes up all threads sleeping on the channel
int wakeup(void *chan);

int sched(cpucontext_t *current_ctx);
void yield();

// Initializes the threads manager
void kthread_init();

// Starts a thread that executes the given function
int kthread_start(void(*func)(void *data), void *data);

#endif
