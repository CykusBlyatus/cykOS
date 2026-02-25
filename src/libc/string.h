#ifndef CYKOS_STRING_H
#define CYKOS_STRING_H

#include <stddef.h>

// Copying

void *memcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);

// Concatenation

char *strcat(char *dest, const char *src);
char *strncat(char *dest, const char *src, size_t n);

// Comparison

int memcmp(const void *ptr1, const void *ptr2, size_t n);
int strcmp(const char *str1, const char *str2);
int strcoll(const char *str1, const char *str2);
int strncmp(const char *str1, const char *str2, size_t n);
size_t strxfrm( char *dest, const char *src, size_t n);

// Searching

void *memchr(const void *mem, int c, size_t n);
char *strchr(const char *str, int c);
size_t strcspn(const char *str, const char *chars);
char *strpbrk(const char *str, const char *chars);
char *strrchr(const char *str, int c);
size_t strspn(const char *str, const char *chars);
char *strstr (const char *str1, const char *str2);
char *strtok (char *str, const char *delimiters);

// Other

void *memset(void *mem, int c, size_t n);
size_t strlen(const char *str);
const char *strerror(int errnum);

#endif // CYKOS_STRING_H
