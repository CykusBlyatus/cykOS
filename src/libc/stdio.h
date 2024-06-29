#ifndef CYKOS_STDIO_H
#define CYKOS_STDIO_H

#include <stdarg.h>

int putchar(int c);

// TODO: replace it with fputs in the future
// (according to the standard, fputs doesn't put a '\n' at the end, unlike puts(), which is why this function exists)
int prints(const char *str);

int puts(const char *str);

// WARNING: returns 0 on success instead of bytes transmitted
int printf(const char *format, ...);

// WARNING: returns 0 on success instead of bytes transmitted
int vprintf(const char *format, va_list args);

#endif /* CYKOS_STDIO_H */
