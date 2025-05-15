# 1. What is the heap?
- The heap is a giant memory regino that cfan be shared in a controlled manner.
- You can ask the heap for emmory and tell the heap when you're done with that memory.
- Heap implementations are essentially system memory managers.

## 1.0 Memory in the C Programming Language
- In C, we can point to any address in the RAM memory, regardless if we can access it or not.
- As of right now, if we were to use this C code, it would crash.
```
int main (int argc, char** argv)
{
	char* ptr = (char*)(0x100000);
	ptr[0] = 'A';
}
```
- But in our kernel, it would not, as there are no restrictions set in place yet.
## 1.1. Malloc in C
- Malloc returns a memory address that we can write to (it becomes ours).
- Ensures that any other time our program calls "malloc" it doesn't return a memory address that is unavailable.
- This code would run.
```
int main (int argc, char** argv)
{
	char* ptr = (char*) malloc(50);
	ptr[0] = 'A';
	char* ptr2 = (char*) malloc(50);
	ptr2[0] = 'A';
}
```
## 1.2. Free in C
- Similar to `malloc`, `free` allows us to free the memory in the heap that we previously asked for.
- After being freed, the system marks the memory as available to other processes.
- Next time `malloc` is called, we can safely end up with a previous address that was used.
- This code would run.
```
int main (int argc, char** argv)
{
	char* ptr = (char*) malloc(50);
	free(ptr);
}
```
## 1.3. Limits in a 32-bit protected mode kernel.
- While in protected mode we have certain restrictions, as the processor is in a 32-bit state.
- As we run ni 32 bit mode, we have access only to 32 bit memory addresses, allowing us to addres to a maximum of 4.29G of 4294967296 bytes of RAM regardless of how much system RAM is installed.
# 1.4 Memory of an uninitialized system
- Video memory takes up portions of the RAM.
- Hardware memory takes up portions of the RAM.
- Unused parts of RAM are available to use.
- An array of uninitialized memory is available to us from address "`0x01000000`" which can be a lot or too little, depending on the installed memory.
```ad-note
Address `0xC0000000` is reserved. This means that the memory array we have at address `0x01000000` can give us a maximum of 3.22GB of RAM for a machine with 4GB or more of physically installed memory.
```
# 1.5 Then, the heap is...
- Pointed to an address unused by hardware that is also big enough for us to use.
- THe heap data size can be defined for example, to 100MB of heap memory.
- So long as we have 100MB of memory available, our heap will work fine.
- We'll need a heap implementation to make our haep work properly.
- The heap will be responsible for storing information in our kernel.
- The heap implementation will be responsible for managing this giant chunk of memory that we call the heap.

# 1.6 Simplest possible heap implementation
- Start with a start address and call it a "current address" and point it to somewhere free i.e., `0x01000000`.
- Any call to malloc gets the current address, stores it in a temporary variable called `tmp`.
- Now the current address is incremented by the size provided to `malloc`.
- Temporary variable called `tmp` that contains the allocated address is returned.
- `current_address` now contains the next address for `malloc` to return when `malloc` is called again.
- **Benefits**:
	- Easy to implement.
- **Cons**:
	- Memory can never be released, which may eventually lead to the system being unusable and requiring a reset.
- This is its implementation:
```
void* current-address = (void*)(0x01000000);
void* malloc(int size)
{
	void* tmp = current_address;
	current_address += size;
	return tmp;
}

void* free(void* ptr)
{
	// we cannot free the memory due to its design.
}
```
# 1.7 Our heap implementation
- Will consist of a giant table which describes a giant piece of free memory in the system. This atble will describe which memory is taken, which memory is free and so on. **We will call this the "entry table"**.
- Will have another pointer to a giant piece of free memory, this will be the actual heap data itself that users of `malloc` can use. **We will call this the "data pool"**. If our heap can allocate 100 MB of RAM, then the heap data pool will be of 100 MB in size.
- Our heap implementation will be block based, each address returned from `malloc` will be aligned to 4096 and will at least be 4096 in size.
- If you requested to have `50` bytes of memory, 4096 bytes of memory will be returned to you.

## 1.8 The entry table
- Its composed of an array of 1 byte values that represent an entry in our heap data pool.
- Array size is calculated by taking the heap data pool size and dividing it by our block size of 4096 bytes. We are left with the total number fo entries we need in our array.
- For example:
	- We want a 100MB heap then the math would be:
		- 100MB / 4096 = 25600 bytes in our entry table.
	- If our heap data pool is at address `0x01000000` then entry zero in our table will represent address `0x01000000`.
	- Entry one will represent address `0x01001000`.
	- Entry two will represent address `0x01002000`.
	- `0x1000`: 4096 bytes.
## 1.9 The entry structure
Upper 4 bits are flags.
			Lower 4 bits represent the entry type.

| HAS_N | IS_FIRST | 0   | 0   | ET_3 | ET_2 | ET_1 | ET_0 |
| ----- | -------- | --- | --- | ---- | ---- | ---- | ---- |
- `HAS_N`: Set if the entry to the right of us is part of our allocation.
	- If we allocate 2 blocks of memory, than entry 0 would have the `HAS_N` bit set, and block 1 would not.
- `IS_FIRST`: Set if this is the first entry of our allocation.
- 00: Unused.
- **Each entry byte describes 4096 bytes of data in the heap data pool**.

## 1.10 Entry types
- `HEAP_BLOCK_ENTRY_TAKEN`: The entry is taken and the address cannot be used.
- `HEAP_BLOCK_ENTRY_FREE`: The entry is free and may be used.

## 1.11 Data pool
- It's simply a raw flat array of thousands of millions of bytes that our heap implementation can give to people who need memory.

## 1.12 Malloc example
- First we assume our heap data pool to point to address `0x01000000` .
- We assume our heap is 100MB in size.
- We assume we have 25600 entries in our entry table that describe our 100MB of data in the data pool.
	- 100MB / 4096 = 25600.
### 1.12.1 Memory allocation process
- Take the size from `malloc` and calcyulate how many blocks we need to allocate for this size. If the user asks for "5000" bytes we will need to allocate 8192 bytes because our implementation works on 4096 byte blocks. 8192 is two blocks.
- Check the entry table for the first entry we can find that has a type of `HEAP_BLOCK_TABLE_ENTRY_FREE`, meaning that the 4096 block that this entry represents is free for use.
- Since we require two blocks, we also need to ensure the next entry is also free for use, otherwise we will need to discard the first block we found and look further in our table until we find at least two free blocks that are next to each other.
- Once we have two blocks, we mark those blocks as taken `HEAP_BLOCK_ENTRY_TAKEN`.
- We now reutnr the absolute address that the starting block represents. Calculation:
	- (heap_data_pool_start_address + (block_number * block_size))
### 1.12.2 Finding the total blocks
- Block size: 4096.
- Get the size priovided to `malloc`. For example, value "5000" we then align it to "4096" and we get the value 8192.
```
if (( 5000 % 4096 ) == 0)
{
	return 5000;
}
uint32_t new_val = 5000 - (5000 % 4096);
new_val += 4096;

return new_val;
```
- Now we divide 8192 by our block size of "4096" which gives us `8192 / 4096 = 2`. **Two blocks to allocate**.

### 1.12.3 Finding the two free blocks in the table.

| 0xC1 | 0x81 | 0x81 | 0x01 | 0xC1 | 0x01 | 0x41 | 0x41 | 0x41 | 0x41 |
| ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- |
| 0x41 | 0x41 | 0xC1 | 0x81 | 0x81 | 0x81 | 0x81 | 0x81 | 0x81 | 0x81 |
| 0x81 | 0x01 | 0x00 | 0x00 | 0x41 | 0xC1 | 0x01 | 0x00 | 0x00 | 0x00 |
| 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 |
- `0xC1 | 1100 0001`: Block taken, first block, has more blocks for this allocation.
- `0x41 | 0100 0001`: Block taken, first block, no more blocks for this allocation.
- `0x81 | 1000 0001`: Block taken and we have more blocks for this allocation, we are not the first block for this allocation.
- `0x01 | 0000 0001`: Block taken, we are not the first block for this allocation, no more flobkc for this allocation.
- `0x00 | 0000 0000`: Block free or (lower 4 bits = 0) block free.

We can see in the third row and forth and fifth column that we have two unallocated blocks that we can use. After allocating them, they'll become `0xC1` and `0x01`.

### 1.12.4 Calculating the absolute address for the programmer to use.
The first index that we can use in our heap entry table is the 22 index of our array. The mathematical formula to get the offset is:

`(array_iudex * BLOCK_SIZE)`. In this case:

`(22 * 4096) = 90122` decimal. This is our offset.
If our heap data pool starts at address `0x01000000`, we add the offset.
`initial_address + offset = absolute_address`.
So:
`0x01000000 + 90122d` = `0x101600A`
90122 = 0x1600A

So the absolute address that `malloc` has to return is `0x101600A` or 16,867,338 in decimal. Now the programmer can safely write his 5000 bytes to the address `0x101600A`.

```ad-note
Technically, the programmer will be able to write up to 8192 bytes before overflowing to other people's memory addresses'.
```
## 1.13. Free example.
- Calculate the block number based on the address provided for us to free.
- Go through the entry table starting at the block number we have calcualted, set each entry to `0x00` until we reach the last block of the allocation.
- **We know how many blocks we need to free because the current blockw e are freeing wil now have the `HAS_N` bit set in the entry byte**.

## 1.14. Advantages of our implementation.
- Fast allocation of memory blocks.
- Fast to free blocks of memory.
- Can be written in under 200 lines of code (easy to implement).

## 1.15. Disadvantages of our implementation.
- We allocate in memory blocks, meaning misaligned sizes requested from our heap will result in wasted lost bytes.
- Memory fragmentation is possible.

# 2. Implementing our Heap
this chapter was really long and intense. lots of code was written, so documenting it all will take me a few days. nevertheless, a proper heap, `kmalloc` and `kfree` was implemented!

## 2.1. changes to config.h
we had to add some values to the `config.h` file. these are mostly related to heap size, RAM block size and the heap table address.
```
// 100MB heap size, 1024*1024*100
#define PEACHOS_HEAP_SIZE_BYTES         104857600
// block size
#define PEACHOS_HEAP_BLOCK_SIZE         4096
// heap starting memory address
#define PEACHOS_HEAP_ADDRESS            0x01000000
// table address
#define PEACHOS_HEAP_TABLE_ADDRESS      0x00007E00
```
- `#define PEACHOS_HEAP_SIZE_BYTES         104857600`: this is the total heap size in bytes. this was calculated using the formula (1024\*1024\*MEGS).
- `#define PEACHOS_HEAP_BLOCK_SIZE         4096`: here we define an arbitrary block size. it can be anything, but in our case, we're using 4 kilobyte blocks.
- `#define PEACHOS_HEAP_ADDRESS            0x01000000`: this address is where the heap begins. check [[Heap and memory alloc related notes]] for more info.
- `#define PEACHOS_HEAP_TABLE_ADDRESS      0x00007E00`: this address is where the heap table resides. check [[Heap and memory alloc related notes]] for more info.
## 2.2. status.h
we created a `status.h` file that contains several status codes for exiting and error reporting. I won't explain the header guard, as we've explained it several times before.
```
#ifndef STATUS_H
#define STATUS_H

#define PEACHOS_ALLOK 0
#define EIO 1
#define EINVARG 2
#define ENOMEM 3

#endif
```
- `#define PEACHOS_ALLOK 0`: this is the correct exit value. if return is 0, all is good.
- `#define EIO 1`: this is an error related to input/output issues.
- `#define EINVARG 2`: this is an error related to invalid arguments, like invalid alignment bytes.
- `#define ENOMEM 3`: this is an error returned when during heap allocation, no usable memory is found.

## 2.3. explain heap.c, heap.h
this is the meat of the code. here's where our heap implementation resides, and it is quite extensive. I will map this later on, and when the map its done, i'll put it here; for now, let's see the code piece by piece.

![[Heap structure of execution.canvas|Heap structure of execution]]

### 2.3.1 heap.h
```
#ifndef HEAP_H
#define HEAP_H
#include "config.h"
#include <stdint.h>
#include <stddef.h>

#define HEAP_BLOCK_TABLE_ENTRY_TAKEN    0x01
#define HEAP_BLOCK_TABLE_ENTRY_FREE     0x00

#define HEAP_BLOCK_HAS_NEXT             0b10000000
#define HEAP_BLOCK_IS_FIRST             0b01000000

typedef unsigned char HEAP_BLOCK_TABLE_ENTRY;

struct heap_table
{
        HEAP_BLOCK_TABLE_ENTRY* entries;
        size_t total;
};

struct heap
{
        struct heap_table* table;
        // start address of the heap data pool
        void* saddr;
};

int heap_create(struct heap* heap, void* ptr, void* end, struct heap_table* table);
void* heap_malloc(struct heap* heap, size_t size);
void heap_free(struct heap* heap, void* ptr);

#endif
```

- `#define HEAP_BLOCK_TABLE_ENTRY_TAKEN    0x01`: this it the taken flag (0001) for heap entries that are taken.
- `#define HEAP_BLOCK_TABLE_ENTRY_FREE     0x00`: this is the free flag (0000) for heap entries that are free to use.
- `#define HEAP_BLOCK_HAS_NEXT             0b10000000`: this is the memory flag for heap entries that have one or more blocks in use after the one that's being accessed.
- `#define HEAP_BLOCK_IS_FIRST             0b01000000`: this is the memory flag for heap entries that are the first in their memory allocation.
- `typedef unsigned char HEAP_BLOCK_TABLE_ENTRY;`: here we define a type of `HEAP_BLOCK_TABLE_ENTRY`, which we'll use to define the heap memory entry allocation table. it's and 8 bit value.
- `struct heap_table`: here we define our `heap_table`, which we'll use to describe the heap in use and free.
- `        HEAP_BLOCK_TABLE_ENTRY* entries;`: here the entries are defined.
- `        size_t total;`: and here is the size of each entry.
- `struct heap`: here we define our `heap`, literal heap.
- `        struct heap_table* table;`: here, when we create a `heap`, we point to the `heap_table` structure as a descriptor of the heap itself.
- `        void* saddr;`: here we give the `heap` its starting address. this value will be incremented and decremented depending on heap usage.
- `int heap_create(struct heap* heap, void* ptr, void* end, struct heap_table* table);`: this interface is exposed to allow the kernel the ability to initialize the heap and be able to expose other endpoints, such as `kmalloc` and `kfree` in the future.
- `void* heap_malloc(struct heap* heap, size_t size);`: this function is used to allocate memory, we'll see it later on.
- `void heap_free(struct heap* heap, void* ptr);`: and this function is used to free those memory blocks.

### 2.3.2. heap.c
I will not explain the `heap.c` file from top to bottom, but rather by order of execution. First, let's see how the heap is created. the map I made (and that is shown above) will be very helpful in our process to understand how the heap is created and how it works.

#### 2.3.2.1. Heap creation.
the heap creation process goes from:
- heap_create,
	- heap_validate_alignment,
	- heap_validate_table.
##### 2.3.2.1.1 `heap_create`
This is the main function that will allow us to create the heap. it calls two validation functions: `heap_validate_alignment` and `heap_validate_table`. this will help making sure that the parameters passed by the kernel are correct and work within our implementation.
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

- `int heap_create(struct heap* heap, void* ptr, void* end, struct heap_table* table)`: first we define `heap_create`. this function will be mainly called by the kernel at init time and no one else.
- `        int res = 0;`: here we create `res` with `0` as the initialized variable.
- `        if ( !heap_validate_alignment(ptr) || !heap_validate_alignment(end) )`: we need to check if the values for the beginning and end pointers are valid and see if the operation `ptr % 4096 == 0`. check [[#2.3.2.1.2 `heap_validate_alignment`]]
- `                res = -EINVARG;`: if not, we know that we some of the parameters were wrong.
- `                goto out;`: here wo go to the `out` label and return 0.
- `        memset(heap, 0, sizeof(struct heap));`: here we zero out our heap structure, in case some other memory was there before we claimed it.
- `        heap->saddr = ptr;`: here we set the starting address of the heap. in our case, it's `0x01000000`. (check `config.h`)
- `        heap->table = table;`: here we pass the heap entry table passed by the kernel to the `heap` structure.
- `        res = heap_validate_table(ptr, end, table);`: here we validate whether the table is correct or not. check [[#2.3.2.1.3 `heap_validate_table`]]
- `        if (res < 0)`: if `heap_validate_table` is less than zero, then we know the validation failed.
- `                goto out;`: goto out and return less than zero.
- `        size_t table_size = sizeof(HEAP_BLOCK_TABLE_ENTRY) * table->total;`: here we get the table size, as to be able to `memset` the entries to zero, meaning that they're all unused (`b00000000`)
- `        memset(table->entries, HEAP_BLOCK_TABLE_ENTRY_FREE, table_size);`: here we do the memset.
- `out:`: `out` label.
- `        return res;`: here we just return either: a. `-EINVARG` or b. the validated table.
- `}
##### 2.3.2.1.2 `heap_validate_alignment`
this function helps us verify the alignment of the beginning and ending memory sections of our heap.
```
static bool heap_validate_alignment(void* ptr)
{
        return ((unsigned int) ptr % PEACHOS_HEAP_BLOCK_SIZE) == 0;
}
```
- `static bool heap_validate_alignment(void* ptr)`: we define our function that will take a void pointer; in our case, this takes the sections of memory `ptr` and `end` to be validated.
- `        return ((unsigned int) ptr % PEACHOS_HEAP_BLOCK_SIZE) == 0;`: we return 0 only if the operation `ptr % PEACHOS_HEAP_BLOCK_SIZE` (4096) is zero. this is done so we can have a non floating amount of memory blocks to be used, because if `ptr % PEACHOS_HEAP_BLOCK_SIZE` wasn't 0, then memory would be left out and unused.
##### 2.3.2.1.3 `heap_validate_table`
this functions helps us validate the table formation and whether its a valid heap table or not.
```
static int heap_validate_table(void* ptr, void* end, struct heap_table* table)
{
        int res = 0;

        size_t table_size = (size_t)(end - ptr);
        size_t total_blocks = table_size / PEACHOS_HEAP_BLOCK_SIZE;
        if (table->total != total_blocks)
        {
                res = -EINVARG;
                goto out;
        }

out:
        return res;
}
```
- `static int heap_validate_table(void* ptr, void* end, struct heap_table* table)`: we define the function, it will take the starting address, end address and the `heap_table`.
- `        int res = 0;`: as before, we initialize `res` to zero.
- `        size_t table_size = (size_t)(end - ptr);`: this line calculates the difference between the end and beginning memory addresses and gives us a table size. we need to cast it to `size_t`.
- `        size_t total_blocks = table_size / PEACHOS_HEAP_BLOCK_SIZE;`: here we divide the table size by the block size. for more on the calculations, check  [[Heap and memory alloc related notes]]
- `        if (table->total != total_blocks)`: here we check if the total amount of blocks isn't the total amount calculated within this function. if so, the programmer did something wrong.
- `                res = -EINVARG;`: we set it to minus `EINVARG` and return it.
- `                goto out;`: return `-EINVARG`.
- `out:`: `out` label.
- `        return res;`: or return zero if the function ended correctly.
#### 2.3.2.2. Heap memory allocation process.
Now that we've created out heap, we need to understand how our memory is being allocated. Let's check it out.
##### 2.3.2.2.1 `heap_malloc`
this function will be wrapped by the `kmalloc` call. it's used to allocate memory in the heap.
```
void* heap_malloc(struct heap* heap, size_t size)
{
        size_t aligned_size = heap_align_value_to_upper(size);
        uint32_t total_blocks = aligned_size / PEACHOS_HEAP_BLOCK_SIZE;
        return heap_malloc_blocks(heap, total_blocks);
}
```
- `void* heap_malloc(struct heap* heap, size_t size)`: here we define the function.
- `        size_t aligned_size = heap_align_value_to_upper(size);`: here we need to align the size given by using the modulus operation.
- `        uint32_t total_blocks = aligned_size / PEACHOS_HEAP_BLOCK_SIZE;`: this will give us the total amount of blocks that are needed to be allocated.
- `        return heap_malloc_blocks(heap, total_blocks);`: and this function starts doing the actual allocation.
##### 2.3.2.2.2 `heap_align_value_to_upper`
this function serves the purpose of aligning the `malloc` sizes to their correspondant block size. for example, if the programmer passes `50`, this function will return `4096`.
```
static uint32_t heap_align_value_to_upper(uint32_t val)
{
        if ((val % PEACHOS_HEAP_BLOCK_SIZE) == 0)
        {
                return val;
        }
        val = (val - ( val % PEACHOS_HEAP_BLOCK_SIZE));
        val += PEACHOS_HEAP_BLOCK_SIZE;
        return val;

}
```

- `static uint32_t heap_align_value_to_upper(uint32_t val)`: definition of the function.
- `        if ((val % PEACHOS_HEAP_BLOCK_SIZE) == 0)`: here we calculate the rest of the division between `val` and `PEACHOS_HEAP_BLOCK_SIZE`.
- `                return val;`: if zero, (meaning a multiple of 4096), we return and don't make any changes to `val`.
- `        val = (val - ( val % PEACHOS_HEAP_BLOCK_SIZE));`: otherwise, we take `val` and substract the rest of `val % PEACHOS_HEAP_BLOCK_SIZE`. check [[Heap and memory alloc related notes]] for more info.
- `        val += PEACHOS_HEAP_BLOCK_SIZE;`: here we now have to add the `PEACHOS_HEAP_BLOCK_SIZE` for it to be aligned correctly.
- `        return val;`: and return the value.
##### 2.3.2.2.3 `heap_malloc_blocks`
this function is called by [[#2.3.2.2.1 `heap_malloc`]]. let's see what it does.
```
void* heap_malloc_blocks(struct heap* heap, uint32_t total_blocks)
{
        void* address = 0;
        int start_block = heap_get_start_block(heap, total_blocks);
        if (start_block < 0)
        {
                goto out;
        }

        address = heap_block_to_address(heap, start_block);

        // mark blocks as taken
        heap_mark_blocks_taken(heap, start_block, total_blocks);

out:
        return address;
}
```
- `void* heap_malloc_blocks(struct heap* heap, uint32_t total_blocks)`: here we define the function.
- `        void* address = 0;`: now we set the address to zero.
- `        int start_block = heap_get_start_block(heap, total_blocks);`: now we get the starting block number on our `heap` array. check [[#2.3.2.2.4 `heap_get_start_block`]] for more.
- `        if (start_block < 0)`: if the `heap_get_start_block` returns a value minor to zero, then we have an error (potentially an ENOMEM).
- `                goto out;`: return address 0.
- `        address = heap_block_to_address(heap, start_block);`: here we calculate the block memory address.
- `        heap_mark_blocks_taken(heap, start_block, total_blocks);`: now we the the in use bits (00000001).
- `out:`: out label.
- `        return address;`: returns the address obtained by either `ENOMEM` or `heap_block_to_address`. this will be the address returned in `kmalloc`.
##### 2.3.2.2.4 `heap_get_start_block`
before actually marking out blocks taken, we need to know which blocks we need to mark as taken. let's see how.
```
int heap_get_start_block(struct heap* heap, uint32_t total_blocks)
{
        struct heap_table* table = heap->table;
        int bc = 0;
        int bs = -1;
        for (size_t i = 0; i < table->total; i++)
        {
                if (heap_get_entry_type(table->entries[i]) != HEAP_BLOCK_TABLE_ENTRY_FREE)
                {
                        bc = 0;
                        bs = -1;
                        continue;
                };
                // if this is first block
                if (bs == -1)
                {
                        bs = i;
                };
                bc++;
                if (bc == total_blocks)
                {
                        break;
                };
        }
        if (bs == -1)
        {
                return -ENOMEM;
        };

        return bs;
}
```
- `int heap_get_start_block(struct heap* heap, uint32_t total_blocks)`: function definition.
- `        struct heap_table* table = heap->table;`: here we take the heap table to be checked.
- `        int bc = 0;`: we initialize the block count.
- `        int bs = -1;`: and the start block.
- `        for (size_t i = 0; i < table->total; i++)`: we need to check the entirety of the heap table to check for free blocks. for that, we access `table->total` in this for loop.
- `                if (heap_get_entry_type(table->entries[i]) != HEAP_BLOCK_TABLE_ENTRY_FREE)`: if the current entry doesn't have the free bits (00000001), we...
- `                        bc = 0;`: restart the counter and
- `                        bs = -1;`: restart the block size.
- `                        continue;`: and continue.
- `                if (bs == -1)`: but if the block doesn't have the taken bits, we...
- `                        bs = i;`: the `bs` to the current index in the entry table.
- `                bc++;`: we increment block count after finding a free block.
- `                if (bc == total_blocks)`: and if the block count is equal to the total_blocks needed, we...
- `                        break;`: break!
- `        if (bs == -1)`: but if not, and if `bs` is -1, then we...
- `                return -ENOMEM;`: probably ran out of memory.
- `        return bs;`: return the start block.
##### 2.3.2.2.5 `heap_get_entry_type`
before calculating the block memory address, let's talk about the `heap_get_entry_type` function.
```
static int heap_get_entry_type(HEAP_BLOCK_TABLE_ENTRY entry)
{
        return entry & 0x0f;
}
```
- `static int heap_get_entry_type(HEAP_BLOCK_TABLE_ENTRY entry)`: function definition.
- `        return entry & 0x0f;`: here we get the entry type using bitwise operations. check [[Heap and memory alloc related notes]] for more info. 
##### 2.3.2.2.6 `heap_block_to_address`
after getting the start block, we need to calculate its address in memory. let's see how.
```
void* heap_block_to_address(struct heap* heap, int block)
{
        return heap->saddr + (block * PEACHOS_HEAP_BLOCK_SIZE);
}
```
- `void* heap_block_to_address(struct heap* heap, int block)`: function definition.
- `        return heap->saddr + (block * PEACHOS_HEAP_BLOCK_SIZE);`: here we calculate the starting memory address of the given block of memory. check [[Heap and memory alloc related notes]] for more.

##### 2.3.2.2.7 `heap_mark_blocks_taken`
with the block memory address and the aligned block size, we can start to mark the blocks as taken.
```
void heap_mark_blocks_taken(struct heap* heap, int start_block, int total_blocks)
{
        int end_block = (start_block + total_blocks) - 1;

        HEAP_BLOCK_TABLE_ENTRY entry = HEAP_BLOCK_TABLE_ENTRY_TAKEN | HEAP_BLOCK_IS_FIRST;
        if (total_blocks > 1)
        {
                entry |= HEAP_BLOCK_HAS_NEXT;
        }

        for (int i = start_block; i <= end_block; i++)
        {
                heap->table->entries[i] = entry;
                entry = HEAP_BLOCK_TABLE_ENTRY_TAKEN;
                if (i != end_block - 1)
                {
                        entry |= HEAP_BLOCK_HAS_NEXT;
                }
        }
}
```
- `void heap_mark_blocks_taken(struct heap* heap, int start_block, int total_blocks)`: function definition.
- `        int end_block = (start_block + total_blocks) - 1;`: here we set the ending block.
- `        HEAP_BLOCK_TABLE_ENTRY entry = HEAP_BLOCK_TABLE_ENTRY_TAKEN | HEAP_BLOCK_IS_FIRST;`: now we create a taken and is first entry. by `OR`ing the two values.
- `        if (total_blocks > 1)`: we check the total block variable to validate whether we need a `HAS_N` bit or not.
- `                entry |= HEAP_BLOCK_HAS_NEXT;`: set the `HAS_N` bit if we have more than one block.
- `        for (int i = start_block; i <= end_block; i++)`: here we'll iterate over all the blocks.
- `                heap->table->entries[i] = entry;`: here we set the first entry in the heap table to `IS_FIRST` and `HAS_N`.
- `                entry = HEAP_BLOCK_TABLE_ENTRY_TAKEN;`: now we change the value to only taken.
- `                if (i != end_block - 1)`: and if the block we're currently in isn't the last block...
- `                        entry |= HEAP_BLOCK_HAS_NEXT;`: we set the `HAS_N` bit.

And done. Although it technically isn't a long process, it sure feels like it is. Nevertheless, this implementation is quite simple.
#### 2.3.2.3. Heap memory freeing process.
We've created our heap and created a way to allocate memory into it. Now we need to create a way to free that memory.
##### 2.3.2.3.1 `heap_free`
This is the initial `free` call that will be abstracted later by `kfree`. 
```
void heap_free(struct heap* heap, void* ptr)
{
        heap_mark_blocks_free(heap, heap_address_to_block(heap, ptr));
}
```
- `void heap_free(struct heap* heap, void* ptr)`: function definition.
- `        heap_mark_blocks_free(heap, heap_address_to_block(heap, ptr));`: here we take the `heap` and `ptr` and call the `heap_mark_blocks_free` function.

##### 2.3.2.3.2. `heap_address_to_block`
This function is used to get a block index from a memory address.
```
int heap_address_to_block(struct heap* heap, void* addr)
{

        return ((int)(addr - heap->saddr)) / PEACHOS_HEAP_BLOCK_SIZE;
}
```
- `int heap_address_to_block(struct heap* heap, void* addr)`: function definition.
- `        return ((int)(addr - heap->saddr)) / PEACHOS_HEAP_BLOCK_SIZE;`: here we get the difference between the `addr` and the initial `saddr` and divide it by the BLOCK_SIZE. check [[Heap and memory alloc related notes]] for more info.
##### 2.3.2.3.3 `heap_mark_blocks_free`
this is the function that does the actual job :)
```
void heap_mark_blocks_free(struct heap* heap, int starting_block)
{
        struct heap_table* table = heap->table;
        for (int i = starting_block; i < (int)table->total; i++)
        {
                HEAP_BLOCK_TABLE_ENTRY entry = table->entries[i];
                table->entries[i] = HEAP_BLOCK_TABLE_ENTRY_FREE;
                if (!(entry & HEAP_BLOCK_HAS_NEXT))
                {
                        break;
                }
        }
}
```
- `void heap_mark_blocks_free(struct heap* heap, int starting_block)`: function definition.
- `        struct heap_table* table = heap->table;`: we access the table by creating another table.
- `        for (int i = starting_block; i < (int)table->total; i++)`: here we start iterating over all the blocks.
- `                HEAP_BLOCK_TABLE_ENTRY entry = table->entries[i];`: we get the entry into a maleable variable.
- `                table->entries[i] = HEAP_BLOCK_TABLE_ENTRY_FREE;`: here we set the entry to `FREE` (b00000000)
- `                if (!(entry & HEAP_BLOCK_HAS_NEXT))`: now we check if the current block has the `HAS_N` bit. if it doesn't...
- `                        break;`: we break the loop and all our blocks have been freed.

## 2.4. kheap.c, kheap.h
Now we'll see how the kernel abstracts these functions into stuff that can actually be used, like `kfree` and `kmalloc`.

### 2.4.1. `kheap.c`
```
#include "kheap.h"
#include "heap.h"
#include "config.h"
#include "kernel.h"

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

void* kmalloc(size_t size)
{
        return heap_malloc(&kernel_heap, size);
}

void kfree(void* ptr)
{
        heap_free(&kernel_heap, ptr);
}
```

- `struct heap kernel_heap;`: here the kernel creates it's heap or data pool.
- `struct heap_table kernel_heap_table;`: and here we create the heap table.
- `void kheap_init()`: we define the `kheap_init` that we'll call in our `kernel.c` file later on.
- `        int total_table_entries = PEACHOS_HEAP_SIZE_BYTES / PEACHOS_HEAP_BLOCK_SIZE;`: here we set the total amount of entries by dividing the `SIZE_BYTES` of the heap and the `BLOCK_SIZE` of each block.
- `        kernel_heap_table.entries = (HEAP_BLOCK_TABLE_ENTRY*)(PEACHOS_HEAP_TABLE_ADDRESS);`: here we cast our constant `0x00007E00` to `HEAP_BLOCK_TABLE_ENTRY*`.
- `        kernel_heap_table.total = total_table_entries;`: here we set the total amount of blocks in our heap.
- `        void* end = (void*)(PEACHOS_HEAP_ADDRESS + PEACHOS_HEAP_SIZE_BYTES);`: here we obtain the ending address of our heap.
- `        int res = heap_create(&kernel_heap, (void*)(PEACHOS_HEAP_ADDRESS), end, &kernel_heap_table);`: here we finally create our heap by calling [[#2.3.2.1.1 `heap_create`]].
- `        if (res < 0)`: we check the response, and if less than zero (could be `EINVARG` or `ENONEM`)...
- `                print("failed to create heap\n");`: we fail.
- `void* kmalloc(size_t size)`: now we define the abstraction for the [[#2.3.2.2.1 `heap_malloc`]] function.
- `        return heap_malloc(&kernel_heap, size);`: here we do the call to `heap_malloc` and return the pointer to the address of memory reserved to the `heap` that we asked.
- `void kfree(void* ptr)`: same as above, but now we're calling [[#2.3.2.3.1 `heap_free`]].
- `        heap_free(&kernel_heap, ptr);`: and here we call `heap_free` and clear the bits off of the entry table for them to be used at a later time.
### 2.4.2. `kheap.h`
```
#ifndef KHEAP_H
#define KHEAP_H
#include <stdint.h>
#include <stddef.h>

void kheap_init();
void* kmalloc(size_t size);
void kfree(void* ptr);

#endif
```
- `void kheap_init();`: prototype for `kheap_init` that will be called by `kernel.c`.
- `void* kmalloc(size_t size);`: prototype for `kmalloc` that will be called by `kernel.c`.
- `void kfree(void* ptr);`: prototype for `kfree` that will be called by `kernel.c`.

And with all the pieces combined, we can see our heap in action!

![[Pasted image 20250512005418.png]]
but what about `kfree`?
![[Sin título 5.jpg]]
It's working! *What I failed to mention was that I had to debug the heap implementation because I misread a `1` for an `i` and a `*` for a `+`.*

# 2. Understanding the concept of Paging
- It allows us to remap memory addresses to point to other memory addresses.
- Can be used to provide the illusion we have the maximum amount of RAM installed.
- Can be used to hide memory from other processes.

## 2.1. Remapping memory
- Paging allows us to remap one memory address to another, so `0x100000` could point to `0x200000`.
- Paging works ni 4096 byte block sizes by default. The blocks are called pages.
- When paging is enabled, the `MMU` (Memory Management Unit) will look at your allocated page tables to resolve virtual addresses into physical addresses.
- Paging allows us to pretend memory exists when it does not.

## 2.2. Virtual vs Physical addresses.
 - Virtual addresses are addresses that are not pointing to the address in memory that their value says they are. Virtual address `0x100000` might point to physical address `0x200000` as an example.
 - Physical addresses are absolute addresses in memory whose value points to the same address in memory. For eaxmple, if phyisical address `0x100000` points to `0x100000`, then this is a physical address (duh).
 - Essentially, virtual and physical addresses are just terms we use to explain how a piece of memory is being accessed.
### 2.2.1. Paging illustration
![[Pasted image 20250514220523.png]]
## 2.3. Structure of Paging
- 1024 page directories that point to 1024 page tables.
- 1024 page table entries per page table.
- Each page table entry covers 4096 bytes of memory.
- Each "4096" byte block of memory is called a page.
- 1024 \* 1024 \* 4096 = 4.294.967.296 Bytes / 4GB of addressable memory.
## 2.4. Page directory and page entry structure
- Holds a pointer to a page table.
- Holds attributes.
Check [[Paging Notes]] for the attribute table.

## 2.5. First Page Table Visualized
![[Pasted image 20250514221855.png]]
### 2.5.1. Second Page Table Visualized
![[Pasted image 20250514222058.png]]

## 2.6. Page Fault Exceptions
- The CPU will call the page fault interrupt `0x14` when there was a problem with paging.
	- The exception is invoked:
		- If you access a apge in memory that does nmot have its `P` (Present) bit set.
		- If you access a page that is for supervisor but you aren't a supervisor.
		- If you write to a page that is read-only and you are not supervisor.
## 2.7. Hiding memory from processes.
- If we give each process its own page directory table, then we can map the memory for the process however we want it to be. We can make it so the process can only see itself.
- Hiding memory can be achieved by switching the page directories when moving between processes.
- All processes can access the same virtual memory addresses but they will point to different physical addresses.
## 2.8 Illusion of more memory.
- We can pretend we have the maximum amount of memory even if we do not.
- This is achieved by creating page tables that are not present. Once a process accesses this non-present address, a page fault will occur. We can then load the page back into memory and the process had no idea of this happening.
- 100MB of system memory can act as if it has access to the full 4GB on a 32 bit architecture.
