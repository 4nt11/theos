#ifndef STRING_H
#define STRING_H
#include <stddef.h>
#include <stdbool.h>

int strlen(const char* ptr);
void* memcpy(void* src, void *dst, size_t size);
bool isdigit(char c);
int tonumericdigit(char c);
int strnlen(const char* ptr, int max);

#endif
