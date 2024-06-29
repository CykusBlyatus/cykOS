#include "stdio.h"
#include "uart.h"

#include <stdint.h>
#include <stddef.h>
#include <limits.h>
#include <ctype.h>

#define to_hex_digit(n) ('0' + (n) + ((n) < 10 ? 0 : 'a' - '0' - 10))

#define UART0_BASE ((volatile uint8_t*)0x10000000)
#define UART0_THR (*UART0_BASE)           // Transmit Holding Register
#define UART0_LSR (*(UART0_BASE + 0x05))  // Line Status Register
#define UART0_LSR_THRE 0x20               // Transmit Holding Register Empty

int putchar(int c) {
    uart_putchar(c);
    return c;
}

int prints(const char *s) {
    while (*s)
        putchar(*s++);
    return 0;
}

int puts(const char *s) {
    prints(s);
    putchar((int)'\n');
    return 0;
}

// Source: https://wiki.osdev.org/RISC-V_Meaty_Skeleton_with_QEMU_virt_board#src/uart/uart.c
// Limited version of vprintf() which only supports the following specifiers:
//
// - d/i: Signed decimal integer
// - u: Unsigned decimal integer
// - o: Unsigned octal
// - x: Unsigned hexadecimal integer
// - X: Unsigned hexadecimal integer (uppercase)
// - c: Character
// - s: String of characters
// - p: Pointer address
// - %: Literal '%'
//
// None of the sub-specifiers are supported for the sake of simplicity.
// The `n` specifier is not supported since that is a major source of
// security vulnerabilities. None of the floating-point specifiers are
// supported since floating point operations don't make sense in kernel
// space
//
// Anyway, this subset should suffice for printf debugging
int vprintf(const char *format, va_list arg) {
    while (*format) {
        if (*format == '%') {
            ++format;
            if (!*format)
                return 0;
            switch (*format) {
                case 'd':
                case 'i':
                {
                    int n = va_arg(arg, int);
                    if (n == INT_MIN) {
                        prints("-2147483648");
                        break;
                    }
                    if (n < 0) {
                        putchar('-');
                        n = ~n + 1;
                    }
                    char lsh = '0' + n % 10;
                    n /= 10;
                    char buf[9];
                    char *p_buf = buf;
                    while (n) {
                        *p_buf++ = '0' + n % 10;
                        n /= 10;
                    }
                    while (p_buf != buf)
                        putchar(*--p_buf);
                    putchar(lsh);
                }
                break;

                case 'u':
                {
                    unsigned n = va_arg(arg, unsigned);
                    char lsh = '0' + n % 10;
                    n /= 10;
                    char buf[9];
                    char *p_buf = buf;
                    while (n) {
                        *p_buf++ = '0' + n % 10;
                        n /= 10;
                    }
                    while (p_buf != buf)
                        putchar(*--p_buf);
                    putchar(lsh);
                }
                break;

                case 'o':
                {
                    unsigned n = va_arg(arg, unsigned);
                    char lsh = '0' + n % 8;
                    n /= 8;
                    char buf[10];
                    char *p_buf = buf;
                    while (n) {
                        *p_buf++ = '0' + n % 8;
                        n /= 8;
                    }
                    while (p_buf != buf)
                        putchar(*--p_buf);
                    putchar(lsh);
                }
                break;

                case 'x':
                {
                    unsigned n = va_arg(arg, unsigned);
                    char lsh = to_hex_digit(n % 16);
                    n /= 16;
                    char buf[7];
                    char *p_buf = buf;
                    while (n) {
                        *p_buf++ = to_hex_digit(n % 16);
                        n /= 16;
                    }
                    while (p_buf != buf)
                        putchar(*--p_buf);
                    putchar(lsh);
                }
                break;

                case 'X':
                {
                    unsigned n = va_arg(arg, unsigned);
                    char lsh = to_hex_digit(n % 16);
                    n /= 16;
                    char buf[7];
                    char *p_buf = buf;
                    while (n) {
                        *p_buf++ = to_hex_digit(n % 16);
                        n /= 16;
                    }
                    while (p_buf != buf)
                        putchar(toupper(*--p_buf));
                    putchar(toupper(lsh));
                }
                break;

                case 'c':
                    putchar(va_arg(arg, int));
                break;

                case 's':
                    prints(va_arg(arg, char *));
                break;

                case 'p':
                {
                    prints("0x");
                    size_t ptr = va_arg(arg, size_t);
                    char lsh = to_hex_digit(ptr % 16);
                    ptr /= 16;
                    char buf[15];
                    char *p_buf = buf;
                    while (ptr) {
                        *p_buf++ = to_hex_digit(ptr % 16);
                        ptr /= 16;
                    }
                    while (p_buf != buf)
                        putchar(*--p_buf);
                    putchar(lsh);
                }
                break;

                case '%':
                    putchar('%');
                break;

                default:
                    putchar('%');
                    putchar(*format);
            }
        } else
        putchar(*format);
        ++format;
    }
    return 0;
}

int printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    int retv = vprintf(format, args);
    va_end(args);
    return retv;
}
