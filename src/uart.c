#include "uart.h"
#include <stdint.h>

#define UART0_BASE ((volatile uint8_t*)0x10000000)

#define UART0_REG_RBR (*(UART0_BASE + 0x00)) // Receiver Buffer Register
#define UART0_REG_THR (*(UART0_BASE + 0x00)) // Transmit Holding Register
#define UART0_REG_IER (*(UART0_BASE + 0x01)) // Interrupt Enable Register
#define UART0_REG_LSR (*(UART0_BASE + 0x05)) // Line Status Register

#define UART_LSR_THRE 0x20 // Transmit Holding Register Empty
#define UART_LSR_DR (1 << 0) // Data Ready

#define UART_IER_RDA (1 << 0) // Received Data Available Interrupt Enable

void uart_init(void) {
    UART0_REG_IER = UART_IER_RDA; // Enable receive data available interrupt
}

void uart_putchar(int c) {
    while (!(UART0_REG_LSR & UART_LSR_THRE));  // Wait until the UART is ready to transmit
    UART0_REG_THR = c;
}

char uart_read(void) {
    while (!(UART0_REG_LSR & UART_LSR_DR));
    return (char)UART0_REG_RBR;
}
