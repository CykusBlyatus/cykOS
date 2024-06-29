#include <stdio.h>
#include "uart.h"
#include "syscon.h"
#include "interrupts.h"
#include "plic.h"

int main() {
    /*
    uart_init();
    plic_set_priority(UART0_IRQ, 1);
    plic_enable_interrupt(UART0_IRQ);
    hw_enable_interrupts();
    */

    int x = 5;
    printf("it's cockin time (btw x = %d and &x = %p)\n", x, &x);

    char c;
    while(1) {
        c = uart_read();
        if (c == 27) // ESC
            break;
        putchar(c);
    }

    poweroff();
}
