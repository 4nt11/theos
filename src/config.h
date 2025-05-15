#ifndef CONFIG_H
#define CONFIG_H

// GDT code segment
#define KERNEL_CODE_SELECTOR 		0x08

// GDT data segment
#define KERNEL_DATA_SELECTOR 		0x10

// OS total amount of interrupts
#define PEACHOS_TOTAL_INTERRUPTS 	512

// 100MB heap size, 1024*1024*100
#define PEACHOS_HEAP_SIZE_BYTES 	104857600

// block size
#define PEACHOS_HEAP_BLOCK_SIZE 	4096

// heap starting memory address
#define PEACHOS_HEAP_ADDRESS 		0x01000000

// table address
#define PEACHOS_HEAP_TABLE_ADDRESS 	0x00007E00

#endif
