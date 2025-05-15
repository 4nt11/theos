#include "paging.h"
#include "memory/heap/kheap.h"

// first we need to map the memory in a linear fashion. 0x1000 = 0x1000.
struct paging_4gb_chunk* paging_new_4gb(uint8_t flags)
{
	uint32_t* directory = kzalloc(sizeof(uint32_t) * PAGING_TOTAL_ENTRIES_PER_TABLE);
	int offset = 0;
	for (int i = 0; i < PAGING_TOTAL_ENTRIES_PER_TABLE; i++)
	{
		uint32_t* entry = kzalloc(sizeof(uint32_t) * PAGING_TOTAL_ENTRIES_PER_TABLE);
		for (int b = 0; b < PAGING_TOTAL_ENTRIES_PER_TABLE; b++)
		{
			entry[b] = (offset + (b * PAGING_PAGE_SIZE) | flags);
		}
		offset += (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE);
		directory[i] = (uint32_t) entry | flags | PAGING_IS_WRITEABLE;
	}
	struct paging_4gb_chunk* chunk_4gb = kzalloc(sizeof(struct paging_4gb_chunk));
	chunk_4gb -> directory_entry = directory;

	return chunk_4gb;
}
