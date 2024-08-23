#ifndef CYKOS_ANSI_COLORS_H
#define CYKOS_ANSI_COLORS_H

#define ANSI_BLACK       "\033[30m"     /*!< Black color */
#define ANSI_RED         "\033[31m"     /*!< Red color */
#define ANSI_GREEN       "\033[32m"     /*!< Green color */
#define ANSI_YELLOW      "\033[33m"     /*!< Yellow color */
#define ANSI_BLUE        "\033[34m"     /*!< Blue color */
#define ANSI_MAGENTA     "\033[35m"     /*!< Magenta color */
#define ANSI_CYAN        "\033[36m"     /*!< Cyan color */
#define ANSI_WHITE       "\033[37m"     /*!< White color */
#define ANSI_RESET       "\033[0m"      /*!< Reset */

#define ANSI_FG(code) "\033[38;5;"#code"m" /*!< @brief Sets the foreground color. The argument must be a literal number. */
#define ANSI_BG(code) "\033[48;5;"#code"m" /*!< @brief Sets the background color. The argument must be a literal number. */

#endif /* CYKOS_ANSI_COLORS_H */
