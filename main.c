#define UART0_BASE ((volatile unsigned char*)0x10000000)
#define UART0_THR (*(UART0_BASE + 0x00))  // Transmit Holding Register
#define UART0_LSR (*(UART0_BASE + 0x05))  // Line Status Register
#define UART0_LSR_THRE 0x20               // Transmit Holding Register Empty

void uart_init() {
    // idfk what to do here yet
}

int putchar(int c) {
    while (!(UART0_LSR & UART0_LSR_THRE));  // Wait until the UART is ready to transmit
    UART0_THR = c;
    return 0;
}

int prints(const char *s) {
    while (*s)
        putchar(*s++);
    return 0;
}

int main() {
    const char str[] = "it's cockin time";
    uart_init();
    prints(str);

    while (1) {
        putchar('!');
        asm volatile ("wfi"); // make it get stuck here (otherwise it will consume CPU)
    }
}
