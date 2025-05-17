#include "memory.h"
//#include "memory/heap/kheap.h"

void* memset(void* ptr, int c, size_t size)
{
	char* c_ptr = (char*) ptr;
	for (int i = 0; i < size; i++)
	{
		c_ptr[i] = (char) c;
	}
	return ptr;
}

void* memcpy(void* src, void *dst, size_t size)
{
	char* src_ptr = (char*)src;
	char* dst_ptr = (char*)dst;
	for (int i = 0; i < size; i++)
	{
		dst_ptr[i] = src_ptr[i];
	}
	return dst;
}
