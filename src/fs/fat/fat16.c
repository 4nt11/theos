#include "fat16.h"
#include "status.h"
#include "fs/file.h"
#include "disk/disk.h"
#include "disk/streamer.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "string/string.h"

#define PEACHOS_FAT16_SIGNATURE 0x29
#define PEACHOS_FAT16_FAT_ENTRY_SIZE 0x02
#define PEACHOS_FAT16_BAD_SECTOR 0xFF7
#define PEACHOS_FAT16_UNUSED 0x00

typedef unsigned int FAT_ITEM_TIPE;
#define FAT_ITEM_TYPE_DIRECTORY 0
#define FAT_ITEM_TYPE_FILE 1

// fat dir entry attribute bitmasks
#define FAT_FILE_READ_ONLY 0x01
#define FAT_FILE_HIDDEN 0x02
#define FAT_FILE_SYSTEM 0x04
#define FAT_FILE_VOLUME_LABEL 0x08
#define FAT_FILE_SUBDIRECTORY 0x10
#define FAT_FILE_ARCHIVED 0x20
#define FAT_FILE_DEVICE 0x40
#define FAT_FILE_RESERVED 0x80

struct fat_header_extended
{
	uint8_t drive_number;
	uint8_t win_nt_bit;
	uint8_t signature;
	uint32_t volume_id;
	uint8_t volume_id_string[11];
	uint8_t system_id_string[8];
} __attribute__((packed));

struct fat_header
{
	uint8_t short_jmp_ins[3];
	uint8_t oem_identifier[8];
	uint16_t bytes_per_sector;
	uint8_t sectors_per_cluster;
	uint16_t reserved_sectors;
	uint8_t fat_copies;
	uint16_t root_dir_entries;
	uint16_t number_of_sectors;
	uint8_t media_type;
	uint16_t sectors_per_fat;
	uint16_t sectors_per_track;
	uint16_t number_of_heads;
	uint32_t hidden_sectors;
	uint32_t sectors_big;
} __attribute__((packed));

struct fat_h
{
	struct fat_header primary_header;
	union fat_h_e
	{
		struct fat_header_extended extended_header;
	} shared;
};

struct fat_directory_item
{
	uint8_t filename[8];
	uint8_t ext[3];
	uint8_t attribute;
	uint8_t reserved;
	uint8_t creation_time_tenths_of_a_sec;
	uint16_t creation_time;
	uint16_t creation_date;
	uint16_t last_access;
	uint16_t high_16_bits_first_cluster;
	uint16_t last_mod_time;
	uint16_t last_mod_date;
	uint16_t low_16_bits_first_cluster;
	uint32_t filesize;
} __attribute__((packed));

struct fat_directory
{
	struct fat_directory_item* item;
	int total;
	int sector_pos;
	int ending_sector_pos;
};

struct fat_item
{
	union
	{
		struct fat_directory_item* item;
		struct fat_directory* directory;
	};
	FAT_ITEM_TIPE type;
};

struct fat_item_descriptor
{
	struct fat_item* item;
	uint32_t pos;
};

struct fat_private
{
	struct fat_h header;
	struct fat_directory root_directory;
	// used to stream data clusters
	struct disk_stream* cluster_read_stream;
	// used to stream the FAT table
	struct disk_stream* fat_read_stream;
	struct disk_stream* directory_stream;
};


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

static void fat16_init_private(struct disk* disk, struct fat_private* private)
{
	memset(private, 0, sizeof(struct fat_private));
	private->cluster_read_stream = diskstreamer_new(disk->id);
	private->fat_read_stream = diskstreamer_new(disk->id);
	private->directory_stream = diskstreamer_new(disk->id);
}

int fat16_sector_to_absolute(struct disk* disk, int sector)
{
	return sector * disk->sector_size;
}

int fat16_get_total_items_per_directory(struct disk* disk, uint32_t directory_start_sector)
{
	struct fat_directory_item item;
	struct fat_directory_item empty_item;
	memset(&empty_item, 0, sizeof(empty_item));

	struct fat_private* private = disk->fs_private;

	int res = 0;
	int i = 0;
	int directory_start_pos = directory_start_sector * disk->sector_size;
	struct disk_stream* stream = private->directory_stream;
	if(diskstreamer_seek(stream, directory_start_pos) != PEACHOS_ALLOK)
	{
		res = -EIO;
		goto out;
	}

	while(1)
	{
		if(diskstreamer_read(stream, &item, sizeof(item)) != PEACHOS_ALLOK)
		{
			res = -EIO;
			goto out;
		}
		if (item.filename[0] == 0x00)
		{
			break;
		}
		if (item.filename[0] == 0xE5)
		{
			continue;
		}
		i++;
	}
	res = i;

out:
	return res;
	}

int fat16_get_root_directory(struct disk* disk, struct fat_private* private, struct fat_directory* directory)
{
	int res = 0;
	struct fat_header* primary_header = &private->header.primary_header;
	int root_dir_sector_pos = (primary_header->fat_copies * primary_header->sectors_per_fat) + primary_header->reserved_sectors;
	int root_directory_entries = private->header.primary_header.root_dir_entries;
	int root_directory_size = (root_directory_entries * sizeof(struct fat_directory_item));
	int total_sectors = root_directory_size / disk->sector_size;
	if(root_directory_size % disk->sector_size)
	{
		total_sectors += 1;
	}
	int total_items = fat16_get_total_items_per_directory(disk, root_dir_sector_pos);

	struct fat_directory_item* dir = kzalloc(sizeof(root_directory_size));
	if(!dir)
	{
		res = -ENOMEM;
		goto out;
	}
	struct disk_stream* stream = private->directory_stream;
	if(diskstreamer_seek(stream, fat16_sector_to_absolute(disk, root_dir_sector_pos)) != PEACHOS_ALLOK)
	{
		res = -EIO;
		goto out;
	}
	if(diskstreamer_read(stream, dir, root_directory_size) != PEACHOS_ALLOK)
	{
		res = -EIO;
		goto out;
	}

	directory->item = dir;
	directory->total = total_items;
	directory->sector_pos = root_dir_sector_pos;
	directory->ending_sector_pos = root_dir_sector_pos + (root_directory_size / disk->sector_size);
	 
out:
	return res;
}

int fat16_resolve(struct disk* disk)
{
	int res = 0;
	struct fat_private* fat_private = kzalloc(sizeof(struct fat_private));
	fat16_init_private(disk, fat_private);

	disk->fs_private = fat_private;
	disk->filesystem = &fat16_fs;

	struct disk_stream* stream = diskstreamer_new(disk->id);
	if(!stream)
	{
		res = -ENOMEM;
		goto out;
	}
	if(diskstreamer_read(stream, &fat_private->header, sizeof(fat_private->header)) != PEACHOS_ALLOK)
	{
		res = -EIO;
		goto out;
	}
	
	if (fat_private->header.shared.extended_header.signature != 0x29)
	{
		res = -EFSNOTUS;
		goto out;
	}

	if (fat16_get_root_directory(disk, fat_private, &fat_private->root_directory) != PEACHOS_ALLOK)
	{
		res = -EIO;
		goto out;
	}

out:
	if(stream)
	{
		diskstreamer_close(stream);
	}

	if(res < 0)
	{
		kfree(fat_private);
		disk->fs_private = 0;
	}

	return res;
}

void* fat16_fopen(struct disk* disk, struct path_part* path, FILE_MODE mode)
{
	return 0;
}
