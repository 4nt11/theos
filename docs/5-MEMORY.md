# Description
This page details the state of our memory management systems. As of right now, we only have a basic heap implementation, `malloc` and `free`. 
# Related code
The related code can be found on the following directories:
- src/memory/
- src/memory/memory.c
- src/memory/memory.h
- src/memory/heap/
- src/memory/heap/heap.c
- src/memory/heap/heap.h
- src/memory/heap/kheap.c
- src/memory/heap/kheap.h
# Architecture
As of right now, the memory part of our code only has the heap, `malloc` and `free`. There are a lot of helper functions in `heap.c` that I might not document.
## `memset(void* ptr, int c, size_t size)`
We have a basic implementation of `memset` in the code. It works as expected.
```
void* memset(void* ptr, int c, size_t size)
{
        char* c_ptr = (char*) ptr;
        for (int i = 0; i < size; i++)
        {
                c_ptr[i] = (char) c;
        }
        return ptr;
}
```
## `heap.h`
Before jumping into the `heap.c` code, we need to read and understand the basic structures and constants that our heap uses.
### Constants
We have four basic constants in our header.
```
#define HEAP_BLOCK_TABLE_ENTRY_TAKEN    0x01
#define HEAP_BLOCK_TABLE_ENTRY_FREE     0x00
#define HEAP_BLOCK_HAS_NEXT             0b10000000
#define HEAP_BLOCK_IS_FIRST             0b01000000
```
- HEAP_BLOCK_TABLE_ENTRY_TAKEN
	- Value:
		- `0x01`
	- Description:
		- This value is used to mark an entry as taken or check if the heap entry is taken.
- HEAP_BLOCK_TABLE_ENTRY_FREE
	- Value:
		- `0x00`
	- Description:
		- This value is used to free an entry or to check if the heap entry is free.
- HEAP_BLOCK_HAS_NEXT
	- Value:
		- `0b10000000`
	- Description:
		- This value is used to check if a heap entry has a next entry or to mark it as a multiblock allocation.
- HEAP_BLOCK_IS_FIRST
	- Value:
		- `0b01000000`
	- Description:
		- This value is used to check if we're starting an allocation or to start an allocation.
### Type definitions
We have some basic types for our table entries.
```
typedef unsigned char HEAP_BLOCK_TABLE_ENTRY;
```
- `HEAP_BLOCK_TABLE_ENTRY`
	- Size: 
		- 1 byte.
	- Description:
		- Each bit in this byte has an assigned attribute. See [[#Constants]] for the attributes or [[Heap and memory alloc related notes|the heap notes]] for more information.
### `heap_table`
The `heap_table` structure is the basis for our heap entry table. It serves as a map of memory entries.
```
struct heap_table
{
        HEAP_BLOCK_TABLE_ENTRY* entries;
        size_t total;
};
```
- `entries`
	- Type:
		- `HEAP_BLOCK_TABLE_ENTRY*`
	- Description:
		- This item in our structure will contain all the entries that we'll use in `kheap.c`.
- `total`
	- Type:
		- `size_t`
	- Description:
		- This item contains the total size of our heap.
### `heap`
This is the data pool of our heap implementation.
```
struct heap
{
        struct heap_table* table;
        // start address of the heap data pool
        void* saddr;
};
```
- `table`
	- Type:
		- `struct heap_table*`
	- Description:
		- This item contains a pointer to the heap table.
- `saddr`
	- Type:
		- `void*`
	- Description:
		- This item contains the initial address of the heap, which is `0x01000000` (check [[6. Configuration|config.h]] for more)
## `heap.c`
`heap.c` is really meaty, so we'll go over only the functions that we actually care about. A more detailed documentation can be found over in [[4. The heap and memory allocation|the heap section.]]
## `heap_create(struct heap*, void* ptr, void* end, struct heap_table* table)`
The `heap_create` function is supposed to be called only by kernelspace. It's used to create the initial heap of the kernel.
```
int heap_create(struct heap* heap, void* ptr, void* end, struct heap_table* table)
{
        int res = 0;
        if ( !heap_validate_alignment(ptr) || !heap_validate_alignment(end) )
        {
                res = -EINVARG;
                goto out;
        }
        memset(heap, 0, sizeof(struct heap));
        heap->saddr = ptr;
        heap->table = table;

        res = heap_validate_table(ptr, end, table);
        if (res < 0)
        {
                goto out;
        }

        size_t table_size = sizeof(HEAP_BLOCK_TABLE_ENTRY) * table->total;
        memset(table->entries, HEAP_BLOCK_TABLE_ENTRY_FREE, table_size);

out:
        return res;
}
```
## `heap_malloc(struct heap* heap, size_t size)`
The `heap_malloc` function is supposed to be called only by kernelspace. It's used to allocate stuff.
```
void* heap_malloc(struct heap* heap, size_t size)
{
        size_t aligned_size = heap_align_value_to_upper(size);
        uint32_t total_blocks = aligned_size / PEACHOS_HEAP_BLOCK_SIZE;
        return heap_malloc_blocks(heap, total_blocks);
}
```
## `heap_free(struct heap* heap, void* ptr)`
The `heap_free` function is used to free allocated memory for other processes. It's only supposed to be called by kernelspace.
```
void heap_free(struct heap* heap, void* ptr)
{
        heap_mark_blocks_free(heap, heap_address_to_block(heap, ptr));
}
```
*\*Note: these two functions use a lot of subfunctions to work. All of them are documented over at [[4. The heap and memory allocation|the heap page]] in the docs.*
## `kheap.c`
We've defined some functions and wrappers in the `kheap.c` file.
### `kheap_init`
We make it our responsibility to initialize the heap. This function does exactly that.
```
struct heap kernel_heap;
struct heap_table kernel_heap_table;

void kheap_init()
{
        int total_table_entries = PEACHOS_HEAP_SIZE_BYTES / PEACHOS_HEAP_BLOCK_SIZE;
        kernel_heap_table.entries = (HEAP_BLOCK_TABLE_ENTRY*)(PEACHOS_HEAP_TABLE_ADDRESS);
        kernel_heap_table.total = total_table_entries;

        void* end = (void*)(PEACHOS_HEAP_ADDRESS + PEACHOS_HEAP_SIZE_BYTES);
        int res = heap_create(&kernel_heap, (void*)(PEACHOS_HEAP_ADDRESS), end, &kernel_heap_table);
        if (res < 0)
        {
                print("failed to create heap\n");
        }
}
```
### `kmalloc(size_t size)`
We create a wrapper function over the functions defined in the `heap.c` file. This works as a `malloc` for our kernel.
```
void* kmalloc(size_t size)
{
        return heap_malloc(&kernel_heap, size);
}
```
### `kfree(void* ptr)`
We create a wrapper function over the functions defined in the `heap.c` file. This works as a `free` for our kernel.
```
void kfree(void* ptr)
{
        heap_free(&kernel_heap, ptr);
}
```

