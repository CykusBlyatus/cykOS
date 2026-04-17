#ifndef CYKOS_DEBUG_H
#define CYKOS_DEBUG_H

// #undef DEBUG

#ifdef DEBUG
    #include <stdio.h>
    #include "ansi_colors.h"
    #define debugout stderr
    #define DEBUG_PRINTF_RAW(...) printf(__VA_ARGS__)
    #define DEBUG_PRINTF_COLORS(file_color,line_color,func_color,...) (printf(ANSI_FG(file_color) "%s: " ANSI_FG(line_color) "%d: " ANSI_FG(func_color) "%s: " ANSI_RESET,__FILE__,__LINE__,__func__),printf(__VA_ARGS__),putchar('\n'))
    #define DEBUG_PRINTF_COLORS_NO_NEWLINE(file_color,line_color,func_color,...) (printf(ANSI_FG(file_color) "%s: " ANSI_FG(line_color) "%d: " ANSI_FG(func_color) "%s: " ANSI_RESET,__FILE__,__LINE__,__func__),printf(__VA_ARGS__))
    #define DEBUG_PUT(x) x
#else
    #define DEBUG_PRINTF_RAW(...) ((void)0)
    #define DEBUG_PRINTF_COLORS(...) ((void)0)
    #define DEBUG_PRINTF_COLORS_NO_NEWLINE(...) ((void)0)
    #define DEBUG_PUT(x)
#endif

#define DEBUG_WARN(...) DEBUG_PRINTF_COLORS(214,130,214,__VA_ARGS__)
#define DEBUG_WARN_NO_NEWLINE(...) DEBUG_PRINTF_COLORS_NO_NEWLINE(214,130,214,__VA_ARGS__)
#define DEBUG_ERROR(...) DEBUG_PRINTF_COLORS(160,124,160,__VA_ARGS__)
#define DEBUG_ERROR_NO_NEWLINE(...) DEBUG_PRINTF_COLORS_NO_NEWLINE(160,124,160,__VA_ARGS__)
#define DEBUG_INFO(...) DEBUG_PRINTF_COLORS(45,37,45,__VA_ARGS__)
#define DEBUG_INFO_NO_NEWLINE(...) DEBUG_PRINTF_COLORS_NO_NEWLINE(45,37,45,__VA_ARGS__)
#define DEBUG_SUCCESS(...) DEBUG_PRINTF_COLORS(28,22,28,__VA_ARGS__)
#define DEBUG_SUCCESS_NO_NEWLINE(...) DEBUG_PRINTF_COLORS_NO_NEWLINE(28,22,28,__VA_ARGS__)

#endif /* CYKOS_DEBUG_H */
