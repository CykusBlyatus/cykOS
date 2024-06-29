#include "uart.h"
#include <stdint.h>

#define UART0_BASE ((volatile uint8_t*)0x10000000)
#define UART0_THR (*UART0_BASE)           // Transmit Holding Register
#define UART0_LSR (*(UART0_BASE + 0x05))  // Line Status Register
#define UART0_LSR_THRE 0x20               // Transmit Holding Register Empty

void uart_init(void) {
    // idfk what to do here yet
}

void uart_putchar(int c) {
    while (!(UART0_LSR & UART0_LSR_THRE));  // Wait until the UART is ready to transmit
    UART0_THR = c;
}
