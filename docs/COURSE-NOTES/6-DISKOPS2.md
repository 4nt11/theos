# 1. Implementing the VFS Fread Function
So... now we'll start writting the code to actually read the contents of a file.
## 1.1. `file.h`
We've made some changes to the filesystem structure and added a function pointer.
```
#include <stdint.h>
typedef int (*FS_READ_FUNCTION)(struct disk* disk, void* private, uint32_t size, uint32_t nmemb, char* out);

struct filesystem
{
        // fs should return 0 from resolve if the disk is using its fs.
        FS_RESOLVE_FUNCTION resolve;
        FS_OPEN_FUNCTION open;
        FS_READ_FUNCTION read;
        char name[20];
};
```
- `#include <stdint.h>`: we've included this header file.
- `typedef int (*FS_READ_FUNCTION)(struct disk* disk, void* private, uint32_t size, uint32_t nmemb, char* out);`: this is a function pointer declaration. we've seen this before, when creating the VFS structures. we're making a function type that can be any function that it is assigned to.
- `FS_READ_FUNCTION read;`: and we've made the function pointer part of the filesystem structure.
## 1.2. `file.c`
Now here's the meat.
### 1.2.1. `int fread(void* ptr, uint32_t size, uint32_t nmemb, int fd)`
```
int fread(void* ptr, uint32_t size, uint32_t nmemb, int fd)
{
        int res = 0;
        if (size == 0 || nmemb == 0 ||fd < 1)
        {
                res = -EINVARG;
                goto out;
        }

        struct file_descriptor* descriptor = file_get_descriptor(fd);
        if(!descriptor)
        {
                res = -EINVARG;
                goto out;
        }

        res = descriptor->filesystem->read(descriptor->disk, descriptor->private, size, nmemb, (char*) ptr);
out:
        return res;
}
```
- `int fread(void* ptr, uint32_t size, uint32_t nmemb, int fd)`: function definition. we'll take a pointer, the file size, the nmemb, and the file descriptor or node index.
- `        int res = 0;`: initial return value.
- `        if (size == 0 || nmemb == 0 ||fd < 1)`: here we check for failure conditions. if any of these are met...
- `                res = -EINVARG;`: we return invalid argument.
- `                goto out;`: and go to out.
- `        struct file_descriptor* descriptor = file_get_descriptor(fd);`: here we go looking for the file descriptor via the node index. we've defined this function before, so we won't be looking at it again.
- `        if(!descriptor)`: we check if the descriptor wasn't created successfully. if not...
- `                res = -EINVARG;`: we return invalid argument.
- `                goto out;`: and go to out.
- `        res = descriptor->filesystem->read(descriptor->disk, descriptor->private, size, nmemb, (char*) ptr);`: and now we call the filesystem's implementation of the `fread` funtion. we'll implement this later on.
- `out:`: `out` label.
- `        return res;`: and we return the values.

# 2. Implementing the FAT16 fread function.
Now we'll implement the filesystem fread function. Let's go!

Note: _I was stuck on this for at least a month. I found several issues within the code, all of which I'll detail here and fix it for you guys. There were also some minor issues, such as typos that were inconsequential, as they were mistyped in all instances. Nevertheless, I fixed them too._

## 2.1. `fat16.c`
First, we need to add the prototype of the function at the very top of our file.
```
int fat16_resolve(struct disk* disk);
void* fat16_open(struct disk* disk, struct path_part* path, FILE_MODE mode);
#### new code:
int fat16_read(struct disk* disk, void* descriptor, uint32_t size, uint32_t nmemb, char* out_ptr);
```
- `int fat16_read(struct disk* disk, void* descriptor, uint32_t size, uint32_t nmemb, char* out_ptr);`: we'll take a disk, a file descriptor, a size, an nmemb and the out pointer.

We too need to add the `fat16_read` and `fat16_fopen` to the filesystem struct. This will allow us to call `fat16_read` and `fat16_fopen` from the structure.
```
struct filesystem fat16_fs =
{
    .resolve = fat16_resolve,
    .open = fat16_fopen,
    .read = fat16_read
};
```
- `    .open = fat16_open,`: this is a function pointer to the `fat16_fopen` function.
- `    .read = fat16_read`: and this is another function pointer, but to the `fat16_read` function.


Now we implement `fat16_fread`.

### 2.1.1. `int fat16_read(struct disk* disk, void* descriptor, uint32_t size, uint32_t nmemb, char* out_ptr)`
The code is the following:
```
int fat16_read(struct disk* disk, void* descriptor, uint32_t size, uint32_t nmemb, char* out_ptr)
{
    int res = 0;
    struct fat_file_descriptor* fat_desc = descriptor;
    struct fat_directory_item* item = fat_desc->item->item;
    int offset = fat_desc->pos;
    for (uint32_t i = 0; i < nmemb; i++)
    {
        res = fat16_read_internal(disk, fat16_get_first_cluster(item), offset, size, out_ptr);
        if (ISERR(res))
        {
            goto out;
        }

        out_ptr += size;
        offset += size;
    }

    res = nmemb;
out:
    return res;
}
```

- `int fat16_read(struct disk* disk, void* descriptor, uint32_t size, uint32_t nmemb, char* out_ptr)`: function declaration.
- `    int res = 0;`: we set the initial return value to zero.
- `    struct fat_file_descriptor* fat_desc = descriptor;`: here we setup the FAT file descriptor by the passed descriptor. we can't directly use the `descriptor` descriptor, because it's a void pointer to a descriptor. assigning it a struct type `fat_file_descriptor*`, we'll be able to work with it.
- `    struct fat_directory_item* item = fat_desc->item->item;`: and now we access the item held by the descriptor, which, if our disk was resolved correctly, should be a `fat_directory_item*` struct.
- `    int offset = fat_desc->pos;`: now we set the offset to the `pos`.
- `    for (uint32_t i = 0; i < nmemb; i++)`: and we start iterating through the `nmemb` (number of members) to read.
- `        res = fat16_read_internal(disk, fat16_get_first_cluster(item), offset, size, out_ptr);`: and here the disk is read.
- `        if (ISERR(res))`: if we have any errors in `res`...
- `            goto out;`: we go to out and return the error.
- `        out_ptr += size;`: now we set the `out_ptr` pointer to the size of the object..
- `        offset += size;`: and we increment the `offset` by the size.
- `    res = nmemb;`: and we set `res` to `nmemb`, which should be a number higher than 0.
- `out:`: out label.
- `    return res;`: and we return `res`.

Now, I previously mentioned that I had made a few errors in the code. We'll look at them now.
## 2.2. Fixing bugs!
### 2.2.1. `fat16.c`
#### Bug #1: Mistakenly allocating sizeof instead of actual root directory size in `fat16_get_root_directory`
```
-	struct fat_directory_item* dir = kzalloc(sizeof(root_directory_size));
+	struct fat_directory_item* dir = kzalloc(root_directory_size);
```
This one is simple. I wwas allocating the size of `root_directory_size` (or the size of an `int`) instead of the actual `root_directory_size`.
#### Bug #2: Multiplication instead of adding reads sector 0 instead of intended sector in `fat16_read_internal_from_stream`
```
-	int starting_pos = (starting_sector * disk->sector_size) * offset_from_cluster;
+	int starting_pos = (starting_sector * disk->sector_size) + offset_from_cluster;
```
_**THIS**_ bug held me over from developing the OS FOR WEEKS. Since I'm not extremely familiar with the FAT specs, it took way longer to debug than it should have. The logic explaination of this bug is as follow:
- When getting the start position of the item to read, `offset_from_cluster` will most possibly be zero when working with small files (less than a cluster in size). This means that, when calculating the `starting_pos`, `(starting_sector * disk->sector_size) * offset_from_cluster` was always returning 0, and this meant that I was reading from the sector 0 (the boot sector) instead of the file.
Not fun.
#### Typo #1: `FAT_ITEM_TIPE` instead of `FAT_ITEM_TYPE` in typedef.
```
-	typedef unsigned int FAT_ITEM_TIPE;
+	typedef unsigned int FAT_ITEM_TYPE;
```
### 2.2.2. `string.c`
#### Bug #4: Operands mixed up in `memcpy`
This bug is pretty simple.
```
-	void* memcpy(void* src, void *dst, size_t size)
+	void* memcpy(void* dest, void* src, int len)
```
I just messed up the order of operands in the function. :)
### 2.2.3. `file.c`
#### Bug #4: Reading from disk private data instead of descriptor private data in `fopen`.
```
-	desc->private = disk->fs_private;
+	desc->private = descriptor_private_data;
```
This is another stupid bug I had.

After fixing everything, we're finally able to actually _READ_ from the disk.
![[Pasted image 20250628234032.png]]
# 3. Implementing the VFS `seek` function.

## 3.1. `file.c`
### 3.1.1. `fseek`
This is the body of the VFS `fseek` function.
```
int fseek(int fd, int offset, FILE_SEEK_MODE whence)
{
        int res = 0;
        struct file_descriptor* desc = file_get_descriptor(fd);
        if (!desc)
        {
                res = -EIO;
                goto out;
        }

        res = desc->filesystem->seek(desc->private, offset, whence);

out:
        return res;
}
```

- `int fseek(int fd, int offset, FILE_SEEK_MODE whence)`: function definition. we'll take a file descriptor, an offset and a SEEK mode.
- `        int res = 0;`: here we set the initial return value.
- `        struct file_descriptor* desc = file_get_descriptor(fd);`: here we get the file descriptor from the `int fd` passed onto us.
- `        if (!desc)`: here we check if the descriptor is valid or not. if not...
- `                res = -EIO;`: we return -EIO.
- `                goto out;`: and go to out.
- `        res = desc->filesystem->seek(desc->private, offset, whence);`: now we call the filesystem `fseek` implementation.
- `out:`: `out` label.
- `        return res;`: and we return `res`.
## 3.2. `file.h`
### FS Fseek Function
We need to add the function pointer. This way, the filesystems will be able to define it later on.
```
typedef int (*FS_SEEK_FUNCTION)(void* private, uint32_t offset, FILE_SEEK_MODE  seek_mode);
```
- `typedef int (*FS_SEEK_FUNCTION)(void* private, uint32_t offset, FILE_SEEK_MODE  seek_mode);`: function pointer. same as above.
### Fseek prototype
And finally we add the function prototype.
```
int fseek(int fd, int offset, FILE_SEEK_MODE whence);
```
### Filesystem `fseek` definition.
```
struct filesystem
{
        // fs should return 0 from resolve if the disk is using its fs.
        FS_RESOLVE_FUNCTION resolve;
        FS_OPEN_FUNCTION open;
        FS_READ_FUNCTION read;
        FS_SEEK_FUNCTION seek;
        char name[20];
};
```
And we finally add the function pointer to the filesystem structure.
# 4. Implementing `fstat` VFS function.
## 4.1. `file.h`
First, we need to add the definitions for our `fstat` VFS function.
### 4.1.1 `FILE_STAT_FLAGS`
```
enum
{
        FILE_STAT_READ_ONLY = 0b00000001
};

typedef unsigned int FILE_STAT_FLAGS;
```
- `enum`: 
- `        FILE_STAT_READ_ONLY = 0b00000001`: 
- `typedef unsigned int FILE_STAT_FLAGS;`: 
### 4.1.2. `struct file_stat`
```
struct file_stat
{
        FILE_STAT_FLAGS flags;
        uint32_t filesize;
};
```
- `struct file_stat`: `file_stat` struct.
- `        FILE_STAT_FLAGS flags;`: these are the flags. we only have read only for now.
- `        uint32_t filesize;`: and the file size.
### 4.1.3. Function pointer.
```
typedef int (*FS_STAT_FUNCTION)(struct disk* disk, void* private, struct file_stat* stat);
```
### 4.1.4. Function prototype.
```
int fstat(int fd, struct file_stat* stat);
```
## 4.2. `file.c`
Now we'll define the actual function within our VFS.
### 4.2.1. `fstat`
```
int fstat(int fd, struct file_stat* stat)
{
        int res = 0;
        struct file_descriptor* desc = file_get_descriptor(fd);
        if (!desc)
        {
                res = -EIO;
                goto out;
        }
        res = desc->filesystem->stat(desc->disk, desc->private, stat);

out:
        return res;
}
```
- `int fstat(int fd, struct file_stat* stat)`: function definition. we'll take a file descriptor and a `file_stat` structure.
- `        int res = 0;`: initial return value.
- `        struct file_descriptor* desc = file_get_descriptor(fd);`: we get the file descriptor.
- `        if (!desc)`: we check if it's a valid fd. it not...
- `                res = -EIO;`: we set `-EIO`.
- `                goto out;`: and we return.
- `        res = desc->filesystem->stat(desc->disk, desc->private, stat);`: if the fd is valid, we call the filesystem implementation of `stat`.
- `out:`: out label.
- `        return res;`: and we return.

Now, we'll go over the FAT16 implementation.
# 5. FAT16 Fstat implementation.
## 5.1. `fat16.c`
### 5.1.1. Function prototype.
```
int fat16_fstat(struct disk* disk, void* private, struct file_stat* stat);
```
### 5.1.2. Function pointer in filesystem structure.
```
struct filesystem fat16_fs =
{
        .resolve = fat16_resolve,
        .open = fat16_fopen,
        .read = fat16_read,
        .seek = fat16_seek,
        .stat = fat16_fstat
};
```
### 5.1.3. `fat16_fstat`
```
int fat16_fstat(struct disk* disk, void* private, struct file_stat* stat)
{
        int res = 0;
        struct fat_file_descriptor* desc = (struct fat_file_descriptor*) private;
        struct fat_item* desc_item = desc->item;
        if (desc_item->type != FAT_ITEM_TYPE_FILE)
        {
                res = -EINVARG;
                goto out;
        }
        struct fat_directory_item* ritem = desc_item->item;
        stat->filesize = ritem->filesize;
        stat->flags = 0x00;

        if(ritem->attribute & FAT_FILE_READ_ONLY)
        {
                stat->flags |= FILE_STAT_READ_ONLY;
        }

out:
        return res;
}
```
- `int fat16_fstat(struct disk* disk, void* private, struct file_stat* stat)`: function definition. we'll take a disk, a file descriptor's `private` data and a `file_stat` structure.
- `        int res = 0;`: initial return value.
- `        struct fat_file_descriptor* desc = (struct fat_file_descriptor*) private;`: here we access the descriptor's private data.
- `        struct fat_item* desc_item = desc->item;`: here we access the previous descriptor's item `fat_item` structure.
- `        if (desc_item->type != FAT_ITEM_TYPE_FILE)`: now we check if the descriptor's type is of a file. if not...
- `                res = -EINVARG;`: we return invalid argument.
- `                goto out;`: and go to out.
- `        struct fat_directory_item* ritem = desc_item->item;`: now we access the item itself using a `fat_directory_item*` structure.
- `        stat->filesize = ritem->filesize;`: and we set the filesize using the previously created `ritem`.
- `        stat->flags = 0x00;`: now we initialize the flags of the `stat`.
- `        if(ritem->attribute & FAT_FILE_READ_ONLY)`: here we check if the attributes match. if so...
- `                stat->flags |= FILE_STAT_READ_ONLY;`: we add the `FILE_STAT_READ_ONLY` bitmask to the `stat->flags` member of the struct.
- `out:`: out label
- `        return res;`: and we return res.
Simple. We're almost done with our VFS/FAT16 implementation.
# 6. Implementing the VFS fclose function.

## 6.1. `file.h`
### 6.1.1. Function pointer
```
typedef int (*FS_CLOSE_FUNCTION)(void* private);
```
### 6.1.2. Filesystem structure update
```
struct filesystem
{
	...
        FS_CLOSE_FUNCTION close;
        ...
};
```
### 6.1.3. Function prototype
```
int fclose(int fd);
```
## 6.2. `file.c`
### 6.2.1. `int fclose(int fd)`
```
int fclose(int fd)
{
        int res = 0;
        struct file_descriptor* descriptor = file_get_descriptor(fd);
        if(!descriptor)
        {
                res = -EIO;
                goto out;
        }
        res = descriptor->filesystem->close(descriptor->private);
        if(res == PEACHOS_ALLOK)
        {
                file_free_descriptor(descriptor);
        }

out:
        return res;
}
```
- `int fclose(int fd)`: function definition. we'll just take a file descriptor.
- `        int res = 0;`: we set the initial return value.
- `        struct file_descriptor* descriptor = file_get_descriptor(fd);`: here we get the file descriptor.
- `        if(!descriptor)`: we check if it's a valid descriptor. if not...
- `                res = -EIO;`: we return `-EIO`
- `                goto out;`: and go to out.
- `        res = descriptor->filesystem->close(descriptor->private);`: now we call the filesystem implementation of `close`.
- `        if(res == PEACHOS_ALLOK)`: we check if `res` returned a positive value. if so...
- `                file_free_descriptor(descriptor);`: we now free the descriptor from the VFS.
- `out:`: out label
- `        return res;`: and we return.
### 6.2.2 `static void file_free_descriptor(struct file_descriptor* desc)`
When freeing, we cannot just rely on the filesystem to free the descriptor, because the filesystem only frees the private data, but not the descriptor itself. We have to do that.
```
static void file_free_descriptor(struct file_descriptor* desc)
{
        file_descriptors[desc->index-1] = 0x00;
        kfree(desc);
}
```
- `static void file_free_descriptor(struct file_descriptor* desc)`: function definition. we'll take a `file_descriptor*` structure.
- `        file_descriptors[desc->index-1] = 0x00;`: here we set the `file_descriptors` array at index `desc->index-1` to `0x00`, effectively freeing it.
- `        kfree(desc);`: and we now free the memory associated with the descriptor itself.

We're almost done! Now we only have to implement the FAT16 fclose call and we'll we ready to go.
# 7. Implementing the FAT16 `fclose` function.
## 7.1. `fat16.c`
### 7.1.1. Function prototype.
```
int fat16_close(void* private);
```
### 7.1.2. Modifying our filesystem.
```
struct filesystem fat16_fs =
{
	...
	.close = fat16_close
	...
};
```
### 7.1.3. `static void fat16_free_file_descriptor(struct fat_file_descriptor* desc)`
```
static void fat16_free_file_descriptor(struct fat_file_descriptor* desc)
{
        fat16_fat_item_free(desc->item);
        kfree(desc);
}
```
This function is quite simple. We leverage the previously created `fat_item_free` function to free the memory used by the item itself. After that, we just free the descriptor by calling `kfree`.
### 7.1.4. `int fat16_close(void* private)`
```
int fat16_close(void* private)
{
        fat16_free_file_descriptor((struct fat_file_descriptor*) private);
        return 0;
}
```
This function too is simmple. We just call our previously created `fat16_free_file_descriptor`.

And our filesystem is ready. Not production ready, of course. We can't even write files. But it'll work for now.
