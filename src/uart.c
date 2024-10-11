#include "uart.h"
#include "uart_macros.h"
#include <stdint.h>
#include "syscon.h"

void uart_init(void) {
    UART0_IER = 0; // disable interrupts

    // set baud rate to 38.4K
    UART0_LCR = UART_LCR_BAUD;
    UART0_REG(0) = 3;
    UART0_REG(1) = 0;

    UART0_LCR = UART_LCR_BPC(8) | UART_LCR_PAR_NONE; // set word length to 8 bits, no parity

    UART0_FCR = UART_FCR_CLR | UART_FCR_ENABLE; // clear and enable fifo

    UART0_IER = UART_IER_RECV; // Enable receive data available interrupt
}

void uart_putchar(int c) {
    for (uint64_t i = 0; i < 1000000; ++i) { // Wait until the UART is ready to transmit
        if (UART0_LSR & UART_LSR_THRE) {
            UART0_THR = c;
            return;
        }
    }
    poweroff(); // yeah.
}

char uart_read(void) {
    while (!(UART0_LSR & UART_LSR_RECV));
    return (char)UART0_RBR;
}
