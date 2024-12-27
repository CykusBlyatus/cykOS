#include <string.h>

typedef unsigned long long chunk_t;

// Copying

void *memcpy(void *const dest, const void *src, const size_t n) {
    void *p = dest;
    for (size_t i = 0; i < n / sizeof(chunk_t); ++i, p += sizeof(chunk_t), src += sizeof(chunk_t))
        *(chunk_t*)p = *(const chunk_t*)src;
    for (size_t i = 0; i < n % sizeof(chunk_t); ++i, ++p, ++src)
        *(char*)p = *(const char*)src;
    return dest;
}

void *memmove(void *const dest, const void *src, const size_t n) {
    if (dest < src)
        return memcpy(dest, src, n);
    void *p = dest - 1;
    for (size_t i = 0; i < n / sizeof(chunk_t); ++i, p -= sizeof(chunk_t), src -= sizeof(chunk_t))
        *(chunk_t*)p = *(const chunk_t*)src;
    for (size_t i = 0; i < n % sizeof(chunk_t); ++i, --p, --src)
        *(char*)p = *(const char*)src;
    return dest;
}

char *strcpy(char *const dest, const char *src) {
    char *d = dest;
    do *(d++) = *(src++); while (*src != '\0');
    return dest;
}

char *strncpy(char *const dest, const char *src, const size_t n) {
    char *d = dest;
    for (size_t i = 0; i < n; ++i) {
        *(d++) = *(src++);
        if (*src == '\0')
            return dest;
    }
    return dest;
}

// Concatenation

char *strcat(char *const dest, const char *src) {
    strcpy(strchr(dest, '\0'), src);
    return dest;
}

char *strncat(char *dest, const char *src, size_t n) {
    strncpy(strchr(dest, '\0'), src, n);
    return dest;
}

// Comparison

int memcmp(const void *ptr1, const void *ptr2, size_t n);

int strcmp(const char *str1, const char *str2);

int strcoll(const char *str1, const char *str2);

int strncmp(const char *str1, const char *str2, size_t n);

size_t strxfrm( char *dest, const char *src, size_t n);

// Searching

void *memchr(const void *mem, int c, size_t n) {
    const void *end = mem + n;
    for (; mem != end; ++mem) {
        if (*(const char*)mem == c)
            return (void*)mem;
    }
    return NULL;
}

char *strchr(const char *str, int x) {
    char c = x;
    while (1) {
        if (*str == c)
            return (char*)str;
        else if (*str == '\0')
            return NULL;
        ++str;
    }
}

size_t strcspn(const char *str, const char *chars);

char *strpbrk(const char *str, const char *chars);

char *strrchr(const char *str, int c);

size_t strspn(const char *str, const char *chars);

char *strstr (const char *str1, const char *str2);

char *strtok (char *str, const char *delimiters);

// Other

void *memset(void *const mem, int x, size_t n) {
    char c = x;
    char *end = mem + n;
    for (char *ptr = mem; ptr != end; ++ptr)
        *ptr = c;
    return mem;
}

size_t strlen(const char *str) {
    const char *p = str;
    while (*(p++));
    return p - str;
    // return strchr(str, '\0') - str; // infinite recursion warning???
}

const char *strerror(int errnum);
