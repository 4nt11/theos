#ifndef DISK_H
#define DISK_H
#include "fs/file.h"

typedef unsigned int PEACHOS_DISK_TYPE;

// real physical disk
#define PEACHOS_DISK_TYPE_REAL 0

struct disk
{
	PEACHOS_DISK_TYPE type;
	int sector_size;
	struct filesystem* filesystem;
	int id;
	void* fs_private;
};


int disk_read_block(struct disk* idisk, unsigned int lba, int total, void* buf);
struct disk* disk_get(int index);
void disk_search_and_init();

#endif 
