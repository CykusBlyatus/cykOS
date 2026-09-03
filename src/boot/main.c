#define DEBUG
#include <auxiliary/debug.h>

#include <stdio.h>
#include <devices/uart/uart.h>
#include <devices/syscon/syscon.h>
#include <trap/plic.h>
#include <devices/timer/timer.h>
#include <memory/paging.h>
#include <devices/virtio/virtio.h>
#include <memory/memory.h>
#include <proc/thread.h>
#include <proc/lock.h>

extern void trap_handler_s();
int main();

// from _start in startup.S
void start() {
    CSRW("mepc", main); // set "return" address to main
    CSRW("stvec", trap_handler_s);

    // set "previous" privilege mode and enable interrupts
    CSRC("mstatus", CSR_STATUS_MPP);
    CSRS("mstatus", CSR_STATUS_MPP_S);

    CSRS("sstatus", CSR_STATUS_SIE);

    CSRW("medeleg", 0xffff); // delegate all exceptions to S-mode
    CSRW("mideleg", 0xffff); // delegate all interrupts to S-mode
    // CSRS("mie", CSR_IEIP_SSI | CSR_IEIP_STI | CSR_IEIP_SEI); // enable all S-mode interrupts
    // CSRS("sie", CSR_IEIP_SSI | CSR_IEIP_STI | CSR_IEIP_SEI); // enable all S-mode interrupts

    // Configure physical memory protection to give supervisor mode access to all of physical memory.
    CSRW("pmpaddr0", 0x3fffffffffffff);
    CSRW("pmpcfg0", 0xf);
    CSRW("satp", 0); // disable paging

    // Allow stimecmp for S-mode timer
    CSRS("menvcfg", 1L << (__riscv_xlen-1));
    CSRS("mcounteren", 1L << 1);

    DEBUG_INFO("calling mret");
    asm volatile ("mret"); // jump to to main
}

__attribute__((unused)) static sleeplock_t lock;

void func(__attribute__((unused)) void *c) {
    DEBUG_INFO("called");
    for (int i = 0; i < 2; ++i) {
    // while (1) {
        putchar('\n');
        asm volatile ("wfi");

        // for (int i = 0; i < 100000000; ++i) asm("");
        // putchar(*(char*)c);

        // sleeplock(&lock);
        // puts(__func__);
        // release(&lock);
        // yield();
    }
}

// (Already in Supervisor Mode)
int main() {
    printf("%s called\n", __func__);

    // CSRS("sstatus", CSR_STATUS_FS_INIT); // allow floating-point operations

    pginit();
    kheapinit();
    kthread_init();

    //*
    uart_init();
    plic_set_priority(UART0_IRQ, 1);
    plic_enable_interrupt(HART_CONTEXT(), UART0_IRQ);
    //*/

    plic_set_priority(DISK0_IRQ, 1);
    plic_enable_interrupt(HART_CONTEXT(), DISK0_IRQ);
    disk0_init();

    char *p = kvmalloc(sizeof(*p));
    *p = '1';
    kthread_start(func, p);

    // char c1 = '2';
    // kthread_start(func, &c1);

    // Enable external interrupts
    CSRS("sie", CSR_IEIP_SEI);

    #include <devices/virtio/blk.h>
    #include <string.h>
    u8 in[512], out[512];
    memset(out, 0x31, sizeof(out));
    virtio_blk_rw((u64)out, 0, 1);
    virtio_blk_rw((u64)in, 0, 0);

    const int bytes_per_line = 16;
    DEBUG_INFO("First sector of disk0: ");
    for (u8 *line = in; line != in + sizeof(in); line += bytes_per_line) {
        for (u8 *byte = line; byte != line + bytes_per_line; ++byte) {
            DEBUG_PRINTF_RAW("0x%s%x ", *byte < 0x10 ? "0" : "", *byte);
        }
        DEBUG_PRINTF_RAW("\n");
    }
    DEBUG_PRINTF_RAW("\n");

    //*
    DEBUG_SUCCESS("Entering main thread loop");
    while (1) {
        // printf("Honk shoo...\n");
        // asm volatile ("wfi");

        for (int i = 0; i < 100000000; ++i) asm("");
        putchar('0');

        // for (int i = 0; i < 100000000; ++i) asm("");
        // sleeplock(&lock);
        // puts(__func__);
        // release(&lock);
        // yield();
    }
    //*/

    DEBUG_WARN("reached end of main");
    poweroff();
}
