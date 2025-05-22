#include "string.h"

int strlen(const char* ptr)
{
	int len = 0;
	while(*ptr != 0)
	{
		len++;
		ptr += 1;
	}
	return len;

}

int strnlen(const char* ptr, int max)
{
	int i = 0;
	for(i = 0; i < max; i++)
	{
		if(ptr[i] == 0)
		{
			break;
		}
	}
	return i;
}

bool isdigit(char c)
{
	return c >= 48 && c <= 57;
}

int tonumericdigit(char c)
{
	return c - 48;
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

char* strcpy(char* dest, const char* src)
{
	char *res = dest;
	while(*src != 0)
	{
		*dest = *src;
		src += 1;
		dest += 1;
	}
	*dest = 0x00;
	return res;
}

