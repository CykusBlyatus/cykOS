#ifndef CYKOS_DEBUG_H
#define CYKOS_DEBUG_H

#ifdef DEBUG
    #include <stdio.h>
    #include "ansi_colors.h"
    #define debugout stderr
    #define DEBUG_PRINTF(...) (printf("%s: %d: %s: ", __FILE__, __LINE__, __func__), printf(__VA_ARGS__), putchar('\n'))

    #define DEBUG_PRINTF_COLORS(file_color,line_color,func_color,...) (printf(ANSI_FG(file_color) "%s: " ANSI_FG(line_color) "%d: " ANSI_FG(func_color) "%s: " ANSI_RESET,__FILE__,__LINE__,__func__),printf(__VA_ARGS__),putchar('\n'))
    #define DEBUG_WARN(...) DEBUG_PRINTF_COLORS(214,130,214,__VA_ARGS__)
    #define DEBUG_ERROR(...) DEBUG_PRINTF_COLORS(160,124,160,__VA_ARGS__)
    #define DEBUG_PERROR(str) DEBUG_ERROR("%s: DEBUG_PERROR called but strerror was not implemented at the time this macro was created\n", str)
    #define DEBUG_INFO(...) DEBUG_PRINTF_COLORS(45,37,45,__VA_ARGS__)
    #define DEBUG_SUCCESS(...) DEBUG_PRINTF_COLORS(28,22,28,__VA_ARGS__)
    #define DEBUG_PUT(x) x
#else
    #define DEBUG_PRINTF(...)
    #define DEBUG_PRINTF_COLORS(...)
    #define DEBUG_WARN(...)
    #define DEBUG_ERROR(...)
    #define DEBUG_PERROR(...)
    #define DEBUG_INFO(...)
    #define DEBUG_SUCCESS(...)
    #define DEBUG_PUT(...)
#endif

#endif /* CYKOS_DEBUG_H */
