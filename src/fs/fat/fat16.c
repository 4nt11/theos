#include "fat16.h"
#include "status.h"
#include "fs/file.h"
#include "disk/disk.h"
#include "string/string.h"

void* fat16_fopen(struct disk* disk, struct path_part* path, FILE_MODE mode);
int fat16_resolve(struct disk* disk);

struct filesystem fat16_fs =
{
	.resolve = fat16_resolve,
	.open = fat16_fopen
};

struct filesystem* fat16_init()
{
	strcpy(fat16_fs.name, "FAT16");
	return &fat16_fs;
}

int fat16_resolve(struct disk* disk)
{
	return 0;
}

void* fat16_fopen(struct disk* disk, struct path_part* path, FILE_MODE mode)
{
	return 0;
}
