# Description
This page details the configuration aspects of the kernel.
# Related code
The related code can be found on the following directories:
- src/config.h
# Configurations
We can define several constants and things in this file. Some things are better left untouched, like the selectors.
```
// GDT code segment
#define KERNEL_CODE_SELECTOR            0x08

// GDT data segment
#define KERNEL_DATA_SELECTOR            0x10

// OS total amount of interrupts
#define PEACHOS_TOTAL_INTERRUPTS        512

// 100MB heap size, 1024*1024*100
#define PEACHOS_HEAP_SIZE_BYTES         104857600

// block size
#define PEACHOS_HEAP_BLOCK_SIZE         4096

// heap starting memory address
#define PEACHOS_HEAP_ADDRESS            0x01000000

// table address
#define PEACHOS_HEAP_TABLE_ADDRESS      0x00007E00
```

- `KERNEL_CODE_SELECTOR`
	- Description:
		- This is the memory address of the `CODE_SEG` GDT section.
- `KERNEL_DATA_SELECTOR `
	- Description:
		- This is the memory address of the `DATA_SEG` GDT section.
- `PEACHOS_TOTAL_INTERRUPTS`
	- Description:
		- This constant defines the total amount of interrupts to be initialized in the kernel.
- `PEACHOS_HEAP_SIZE_BYTES`
	- Description:
		- This constant defines the heap size in bytes. The operation `PEACHOS_HEAP_SIZE_BYTES % PEACHOS_HEAP_BLOCK_SIZE` must return 0, otherwise the initialization of the heap will fail with  `-EINVARG`.
- `PEACHOS_HEAP_BLOCK_SIZE`
	- Description:
		- This constant defines the byte size for each block. The operation `PEACHOS_HEAP_SIZE_BYTES % PEACHOS_HEAP_BLOCK_SIZE` must return 0, otherwise the initialization of the heap will fail with  `-EINVARG`.
- `PEACHOS_HEAP_ADDRESS `
	- Description:
		- This constant defines the initial starting address of our heap.
- `PEACHOS_HEAP_TABLE_ADDRESS`
	- Description:
		- This constant defines the location in memory where the heap table will be located.
