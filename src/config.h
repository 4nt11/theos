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

// disk sector size
#define PEACHOS_SECTOR_SIZE 		512

// max fs
#define PEACHOS_MAX_FILESYSTEMS 	12

// max open files
#define PEACHOS_MAX_FILEDESCRIPTORS 	512

// max path
#define PEACHOS_MAX_PATH 108

// number of segments
#define PEACHOS_TOTAL_GDT_SEGMENTS 3

#endif
