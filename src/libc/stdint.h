#ifndef CYKOS_STDINT_H
#define CYKOS_STDINT_H

#include <limits.h>

#if UCHAR_MAX == 0xFF
    typedef signed char int8_t;
    typedef unsigned char uint8_t;
#else
    #error "expected primitive type 'char' to have a size of 1 byte"
#endif

#if USHRT_MAX == 0xFFFF
    typedef short int16_t;
    typedef unsigned short uint16_t;
#else
    #error "expected primitive type 'short' to have a size of 2 bytes"
#endif

#if UINT_MAX == 0xFFFFFFFF
    typedef int int32_t;
    typedef unsigned int uint32_t;
#elif ULONG_MAX == 0xFFFFFFFF
    typedef long int32_t;
    typedef unsigned long uint32_t;
#else
    #error "expected primitive type 'int' or 'long' to have a size of 4 bytes"
#endif

#if ULLONG_MAX == 0xFFFFFFFFFFFFFFFF
    typedef long long int64_t;
    typedef unsigned long long uint64_t;
#elif ULONG_MAX == 0xFFFFFFFFFFFFFFFF
    typedef long int64_t;
    typedef unsigned long uint64_t;
#else
    #error "expected primitive 'long' or 'long long' to have a size of 8 bytes"
#endif

#endif /* CYKOS_STDINT_H */
