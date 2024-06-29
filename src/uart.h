#ifndef CYKOS_UART_H
#define CYKOS_UART_H

void uart_init(void);

void uart_putchar(int c);

char uart_read(void);

#endif /* CYKOS_UART_H */
