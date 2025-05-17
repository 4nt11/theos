#ifndef MEMORY_H
#define MEMORY_H
#include <stddef.h>

void* memset(void* ptr, int c, size_t size);
void* kzmalloc(void* ptr, int c, size_t size);
void* memcpy(void* src, void* dst, size_t size);

#endif
