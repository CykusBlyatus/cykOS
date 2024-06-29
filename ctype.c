#include "ctype.h"

int toupper(int c) {
    return c >= 'a' && c <= 'z' ? c + 'A' - 'a' : c;
}
