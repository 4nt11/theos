#include "file.h"
#include "status.h"
#include "fat/fat16.h"
#include "disk/disk.h"
#include "kernel.h"
#include "memory/memory.h"
#include "string/string.h"
#include "memory/heap/kheap.h"
#include "config.h"

struct filesystem* filesystems[PEACHOS_MAX_FILESYSTEMS];
struct file_descriptor* file_descriptors[PEACHOS_MAX_FILEDESCRIPTORS];

static struct filesystem** fs_get_free_filesystem()
{
	int i = 0;
	for (i = 0; i < PEACHOS_MAX_FILESYSTEMS; i++)
	{
		if(filesystems[i] == 0)
		{
			return &filesystems[i];
		}
	}
	return 0;
}

void fs_insert_filesystem(struct filesystem *filesystem)
{
	struct filesystem** fs;
	fs = fs_get_free_filesystem();
	if(!fs)
	{
		print("no fs!");
		while(1);
	}
	*fs = filesystem;
}

static void fs_static_load()
{
	fs_insert_filesystem(fat16_init());
}

void fs_load()
{
	memset(filesystems, 0, sizeof(filesystems));
	fs_static_load();
}

void fs_init()
{
	memset(file_descriptors, 0, sizeof(file_descriptors));
	fs_load();
}

static int file_new_descriptor(struct file_descriptor** desc_out)
{
	int res = -ENOMEM;
	for (int i = 0; i < PEACHOS_MAX_FILEDESCRIPTORS; i++)
	{
		if (file_descriptors[i] == 0) 
		{
			struct file_descriptor* desc = kzalloc(sizeof(struct file_descriptor));
			desc->index = i+1;
			file_descriptors[i] = desc;
			*desc_out = desc;
			res = 0;
			break;
		}
	}
	return res;
}

static struct file_descriptor* file_get_descriptor(int fd)
{
	if(fd <= 0 || fd >= PEACHOS_MAX_FILEDESCRIPTORS)
	{
		return 0;
	}
	// descriptors start at 1
	int index = fd - 1;
	return file_descriptors[index];
}

struct filesystem* fs_resolve(struct disk* disk)
{
	struct filesystem* fs = 0;
	for (int i = 0; i < PEACHOS_MAX_FILESYSTEMS; i++)
	{
		if (filesystems[i] != 0 && filesystems[i]->resolve(disk) == 0)
		{
			fs = filesystems[i];
			break;
		}
	}

	return fs;
}

FILE_MODE file_get_mode_by_string(const char* str)
{
	FILE_MODE mode = FILE_MODE_INVALID;
	if(strncmp(str, "r", 1) == 0)
	{
		mode = FILE_MODE_READ;
	}
	if(strncmp(str, "w", 1) == 0)
	{
		mode = FILE_MODE_WRITE;
	}
	if(strncmp(str, "a", 1) == 0)
	{
		mode = FILE_MODE_APPEND;
	}

	return mode;

}

int fopen(const char* filename, const char* mode_string)
{
	int res = 0;
	struct path_root* root_path = pathparser_parse(filename, NULL);
	if(!root_path)
	{
		res = -EINVARG;
		goto out;
	}
	if(!root_path->first)
	// if 0:/ and not 0:/file.txt...
	{
		res = -EINVARG;
		goto out;
	}

	struct disk* disk = disk_get(root_path->drive_no);
	// if 1:/...
	if(!disk)
	{
		res = -EIO;
		goto out;
	}
	if(!disk->filesystem)
	{
		res = -EIO;
		goto out;
	}

	FILE_MODE mode = file_get_mode_by_string(mode_string);
	if(mode == FILE_MODE_INVALID)
	{
		res = -EINVARG;
		goto out;
	}
	void* descriptor_private_data = disk->filesystem->open(disk, root_path->first, mode);
	if(ISERR(descriptor_private_data))
	{
		res = ERROR_I(descriptor_private_data);
		goto out;
	}

	struct file_descriptor* desc = 0;
	res = file_new_descriptor(&desc);
	if(res < 0)
	{
		goto out;
	}
	desc->filesystem = disk->filesystem;
	desc->private = disk->fs_private;
	desc->disk = disk;
	res = desc->index;

out:
	// fopen shouldnt return negative values.
	if(res < 0)
	{
		res = 0;
	}
	return res;
}

