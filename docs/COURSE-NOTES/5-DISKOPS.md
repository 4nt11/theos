# 1. Preparing to read from the hard hisk.
## 1.1. PCI IDE Controller
- IDE refers to the electrical specification of cables which connect ATA drives to a device.
- IDE allows up to four disks to be connected.
- There are four types of disks:
	- ATA (Serial): known as SATA; used by modern hard drives.
	- ATA (Parallel): known as PATA; used by hard drives.
	- ATAPI (Serial): Used by modern optical drives.
	- ATAPI (Parallel): Commonly used by optical drives.
- We don't care if the drive is serial or parallel.
## 1.2. Possible IDE Drive Types
- Primary Master Drive.
- Primary Slave Drive.
- Secondary Master Drive.
- Secondary Slave Drive.

We'll be using the I/O operations we've implemented in the past to be able to read the disk sectors.

# 2. Reading from the disk with the ATA Controller.
Let's see the implementation of the LBA driver we made in Assembly in our `boot.asm` file but in C.
## 2.1. `disk.c`
### 2.1.1. `int disk_read_sector(int lba, int total, void* buffer)`
```
int disk_read_sector(int lba, int total, void* buffer)
{
        outb(0x1F6, (lba >> 24) | 0xE0);
        outb(0x1F2, total);
        outb(0x1F3, (unsigned char)(lba & 0xff));
        outb(0x1F4, (unsigned char) lba >> 8);
        outb(0x1F4, (unsigned char) lba >> 16);
        outb(0x1F7, 0x20);

        unsigned short* ptr = (unsigned short*) buffer;

        for (int b = 0; b < total; b++)
        {
                char c = insb(0x1F7);
                while(!(c & 0x08))
                {
                        c = insb(0x1F7);
                }
                // copy from hdd to memory
                for (int i = 0; i < 256; i++)
                {
                        *ptr = insw(0x1F0);
                        ptr++;
                }
        }
        return 0;
}
```
- `int disk_read_sector(int lba, int total, void* buffer)`: function definition. this might look very familiar. check out the `load32` and `ata_lba_read` labels in the `boot.asm` file. here, we're taking an `lba` (disk sector) initial sector (from), a `total` of sectors to read (up to) and a `buffer` in which we'll store the bytes read.
- `        outb(0x1F6, (lba >> 24) | 0xE0);`: here we right shuffle `lba` by 24 bits and `OR` it with `0xE0` and sending this data to the `0x1F6` I/O port.
	- if you look at `boot.asm`, the lines 73 to 76 might be familiar. this is because we're doing exactly that but in C.
- `        outb(0x1F2, total);`: here we send the total amount of sectors to read into the `0x1F2` I/O port.
- `        outb(0x1F3, (unsigned char)(lba & 0xff));`: in here we're sending the lower 8 bits to the `0x1F3` port. check the LBA ATA misc notes for info on this bitwise operation.
- `        outb(0x1F4, (unsigned char) lba >> 8);`: Here we send the `lba` right shifted by 8 to the `0x1F4` port.
- `        outb(0x1F4, (unsigned char) lba >> 16);`: And here we send the `lba` right shifted by 16 to the `0x1F4` port.
- `        outb(0x1F7, 0x20);`: And now we set the command port `0x1F7` to `0x20` or the read sector command.
- `        unsigned short* ptr = (unsigned short*) buffer;`: here we make a pointer to the buffer passed onto us by the caller. we need to cast it to be able to assign stuff to it.
- `        for (int b = 0; b < total; b++)`: here we start a loop. we'll read `total` amount of bytes into the `ptr` buffer pointer.
- `                char c = insb(0x1F7);`: here we read from the `0x1F7` port. we're expecting for...
- `                while(!(c & 0x08))`: bit 0x08. this bit is sent to us by the LBA drive and tells us that it's ready to send the data.
- `                        c = insb(0x1F7);`: here we continue to read and check if the `0x08` bit is set.
- `                for (int i = 0; i < 256; i++)`: if the `0x08` bit is set we go into the reading loop.
- `                        *ptr = insw(0x1F0);`: here we read two bytes into the `*ptr`.
- `                        ptr++;`: and we increment the pointer until `i` is higher than 256.
- `        return 0;`: and we return!

If you didn't understand the code, I wholeheartedly recommed you reading on the `boot.asm` code again on the 3-PROTECTEDMODE page. It'll help you understand this code better.

## 2.2 `disk.h`
For now, we just publish the function in the header file.
```
#ifndef DISK_H
#define DISK_H

int disk_read_sector(int lba, int total, void* buffer);

#endif 
```
# 3. Implementing a disk driver.
Now, what we have already made will help us abstract more things. For example, we can give the programmer an interface to select a drive and read an amount of bytes from it. For now, we'll only have drive zero, but we'll fix that when the time is right.
## 3.1. `disk.h`
We've made some changes to the `disk.h` file. Mainly, we've created some constants and a struct to help us with our drives.
```
typedef unsigned int PEACHOS_DISK_TYPE;

// real physical disk
#define PEACHOS_DISK_TYPE_REAL 0

struct disk
{
        PEACHOS_DISK_TYPE type;
        int sector_size;
};

int disk_read_block(struct disk* idisk, unsigned int lba, int total, void* buf);
struct disk* disk_get(int index);
void disk_search_and_init();
```
- `typedef unsigned int PEACHOS_DISK_TYPE;`: here we create a type definition of `unsigned int` to a DISK_TYPE. it could be a drive, a partition, or something else.
- `#define PEACHOS_DISK_TYPE_REAL 0`: here we define one of our disk types.
- `struct disk`: here we'll create a disk structure that will help us create information and attributes to our disks. for now, it'll be quite simple.
- `        PEACHOS_DISK_TYPE type;`: including its type and...
- `        int sector_size;`: it's sector size.
- `int disk_read_block(struct disk* idisk, unsigned int lba, int total, void* buf);`: function defintion. we'll see this later!
- `struct disk* disk_get(int index);`: function defintion. we'll see this later!
- `void disk_search_and_init();`: function defintion. we'll see this later!
## 3.2. `disk.c`
We've written three functions that will help us abstract the code and make our code more flexible. Let's go over them one by one.
### 3.2.1. `void disk_search_and_init()`
```
struct disk disk;
void disk_search_and_init()
{
        memset(&disk, 0, sizeof(disk));
        disk.type = PEACHOS_DISK_TYPE_REAL;
        disk.sector_size = PEACHOS_SECTOR_SIZE;
}
```
- `struct disk disk;`: we create a structure in which we'll save our disk data.
- `void disk_search_and_init()`: this function will initialize our disks and disk structures.
- `        memset(&disk, 0, sizeof(disk));`: here we zero out the structure.
- `        disk.type = PEACHOS_DISK_TYPE_REAL;`: here we assign a disk type of `PECHOS_DISK_TYPE_REAL`.
- `        disk.sector_size = PEACHOS_SECTOR_SIZE;`: and here we assign the sector size. we haven't seen this constant definition, but know that it is 512.
Pretty simple code! Let's continue.
### 3.2.2. `struct disk* disk_get(int index)`
```
struct disk* disk_get(int index)
{
        if(index != 0)
        {
                return 0;
        }
        return &disk;
}
```
- `struct disk* disk_get(int index)`: here we take a disk index. we're only working with disk ID 0, so it'll just return the disk for now.
- `        if(index != 0)`: here we check if `index` is not zero.
- `                return 0;`: if it's not zero, return 0 as any non zero index is invalid.
- `        return &disk;`: and return a pointer to the disk.
### 3.2.3. `int disk_read_block(struct disk* idisk, unsigned int lba, int total, void* buf)`
```
int disk_read_block(struct disk* idisk, unsigned int lba, int total, void* buf)
{
        if(idisk != &disk)
        {
                return -EIO;
        }
        return disk_read_sector(lba, total, buf);
}
```
- `int disk_read_block(struct disk* idisk, unsigned int lba, int total, void* buf)`: this function will be the new way in which programmers will access the disks. it'll take a disk, an `lba` starting block, a `total` amount of sectors to read and a `buf` in which we'll store the data read.
- `        if(idisk != &disk)`: here we check if the `idisk` is not the same as the disk we already have. this is for testing purposes and will be changed in the future.
- `                return -EIO;`: if not &disk, then return `-EIO`
- `        return disk_read_sector(lba, total, buf);`: and return the read sectors using the already available `disk_read_sector`.
As you can see, we'll now use the `disk_read_block` function directly instead of calling the `disk_read_sector`. 
## 3.3. `config.h`
We've also defined some new constants in our `config.h` file.
```
#define PEACHOS_SECTOR_SIZE             512
```
It's just the sector size. That's it for now!
# 4. What's a filesystem?
- A filesystem is a structure that describes information stored in a disk.
- Disks do not have the concept of a file. They are not aware of them.
- But the operating system does know about files and uses the filesystem to read and write to or from files.
# 4.1. Disks and what we already know.
- Disks are gigantic arrays of data that are split into several sectors.
- Each sector in a disk is given an LBA (Logical Block Address) number.
- But "files" don't exist in the disk in the sense that we know them. They exist as pure data.
## 4.2. Filesystem Structure
- A filesystem contains raw data for files.
- It contains the filesystem structure header which can explani things such as how many files are no the disk, where the root directory is in relation to its sectors and more.
- They way files are laid out on disk is different depending on the filesystem at hand. For example: a "file" will not be the same or have the same structure in FAT32 versus EXT4. They might contain different metadata, different structures and so on.
- Without filesystems, we'd we working with sector numbers and we'd have to write down where our files are in each sector by hand. It'd be very painful, and that's why we do filesystems.
- Operating systems must have a way to read and understand these filesystems. That's why when you format a USB drive to EXT4 format, many Windows versions will say that the disk is "corrupt" (even though it isn't), because it has no idea what an EXT4 filesystem is and thus can't read from it or understand what's going on in it.
## 4.3. FAT16 (File Allocation Table); 16 bits.
- The first sector in this filesystem format is the boot sector on a disk. Fields also exist in this first sector that describe the filesystem, such as how many reserved sectors follow this sector.
- Then follows the reserved sectors (if any) amd these sectors are typically ignored by the filesystem. There's a field in the bootsector that tells the operating system to ignore them. This isn't automatic and we must make our OS ignore them manually.
- Then we have our first file allocation table. This table contains the values that represents which clusters on the disk are taken and which ones are free for use. A cluster is just a certain number of sectors joined together to represent a cluster.
- There might be a second file allocation table, although this one it's optional and depends on whether the FAT6 header in the boot sector is set or not.
- Then would come the root directory. This directory explains what files/directories are in the root directory of the filesystem. Each entry has a relative name that represents the file or directory name, attributes such as read or write permissions, address of the first cluster representing the data on the disk and more.
- And then we have the data region. Our data is here! :)
# 5. Creating a path parser.
A path parser function will divide something like `/mnt/file.txt` into `drive, path, file`. Although it sounds simple (and it is), at first it looks and feels like very complex operations. With time, we'll realize that indeed, these are very simple and logical operations. Before getting into the parser itself, we'll move around some functions and create other ones that'll help us write the parser. Let's do it!
## 5.1. `string.h`
We've created some new code to be able to validate some stuff, like if a given `char` is a number, convert an ASCII number into an number of `int` type (because 1 is not the same as '1') and a couple more. I've also moved the `memcpy` function we saw before in here.
```
#ifndef STRING_H
#define STRING_H
#include <stddef.h>
#include <stdbool.h>

int strlen(const char* ptr);
void* memcpy(void* src, void *dst, size_t size);
bool isdigit(char c);
int tonumericdigit(char c);
int strnlen(const char* ptr, int max);

#endif
```
- `int strlen(const char* ptr);`: although we've already made a `strlen` function, we'll move it in here, as to keep everything well organized.
- `void* memcpy(void* src, void *dst, size_t size);`: and this `memcpy` function previously was in the `memory` section, but i've moved it here, since it makes more sense.
- `bool isdigit(char c);`: this function will help us check is a `char` is a digit or not.
- `int tonumericdigit(char c);`: this function will help us convert an ASCII digit into an actual number.
- `int strnlen(const char* ptr, int max);`: this function will do a comparison between the lenght of `ptr` and `max`, and it will return different values if `ptr` is higher or lower than `max`.
## 5.2. `string.c`
Now, let's go over the actual code.
### 5.2.1. `int strlen(const char* ptr);`
```
int strlen(const char* ptr)
{
        int len = 0;
        while(*ptr != 0)
        {
                len++;
                ptr += 1;
        }
        return len;
}
```
- `int strlen(const char* ptr)`: function definition. we'll take a `const char* ptr`.
- `        int len = 0;`: here we declare an initial `len` variable which we'll represent the lenght of the `ptr` passed on to us.
- `        while(*ptr != 0)`: here we check if `*ptr` is NULL terminated or the end of the pointer.
- `                len++;`: if not, we increment the lenght.
- `                ptr += 1;`: and we also increment the pointer position.
- `        return len;`: and we return the lenght.
### 5.2.2. `void* memcpy(void* src, void *dst, size_t size);`
We won't go over the `memcpy` code, as it has not changed. I just moved it here.
### 5.2.3. `bool isdigit(char c);`
```
bool isdigit(char c)
{
        return c >= 48 && c <= 57;
}
```
- `bool isdigit(char c)`: here we make a function that will take a `char c`. we will check its ASCII value and determine if it is between 48 and 57, which are integer numbers (check the ASCII table).
- `        return c >= 48 && c <= 57;`: here we do a conditional return and check if `c` is higher or equal to 48 (which is a numeric `0`) and if it is lower or equal to 57 (which is numeric `9`). if true, we return true, if not, we return false (1=true, 0=false).
### 5.2.4. `int tonumericdigit(char c);`
```
int tonumericdigit(char c)
{
        return c - 48;
}
```
- `int tonumericdigit(char c)`: function definition. we'll take a `char c` that is supposed to be an ASCII digit.
- `        return c - 48;`: here we make a conversion from ASCII digits to decimal digits. take ASCII value 50, which is `2`. 50-48 would equal to 2, which is, well, integer 2. in the ASCII table, `2` is `start of text`, but that doesn't matter to us, because we want the raw decimal value.
### 5.2.5. `int strnlen(const char* ptr, int max);`
```
int strnlen(const char* ptr, int max)
{
        int len = 0;
        for(int i = 0; i < max; i++)
        {
                if(ptr[i] == 0)
                {
                        break;
                }
        }
        return len;
}
```
- `int strnlen(const char* ptr, int max)`: as described before, we'll take a `ptr` and a `max` value and we'll use is to count and read a `max` amount of characters from `ptr` and return the `len`.
- `        int i = 0;`: here we initialize `i` as integer to `0`.
- `        for(i = 0; i < max; i++)`: here we start to go over the `ptr` items with `i`.
- `                if(ptr[i] == 0)`: we check if `ptr[i]` is `0` (NULL terminator), and if so...
- `                        break;`: break. if not, keep counting and increment `i`.
- `        return len;`: and return `len`:
This is all very simple. Because this, although it will help us make the path parser, isn't the path parser itself :) Let's go over the changes made to `memory.c` before getting our hands ***really*** dirty.
## 5.3. `memory.h`
We've added a prototype for `memcmp` and deleted some stuff that should've never been there, like `kzmalloc` and `memcpy`.
```
...
int memcmp(void* s1, void* s2, int count);
...
```
We'll go over `memcmp` right now.
## 5.4. `memory.c`
### 5.4.1. `int memcmp(void* s1, void* s2, int count)`
```
int memcmp(void* s1, void* s2, int count)
{
        char* c1 = s1;
        char* c2 = s2;

        while(count-- > 0)
        {
                if (*c1++ != *c2++)
                {
                        return c1[-1] < c2[-1] ? -1 : 1;
                }
        }
        return 0;
}
```
- `int memcmp(void* s1, void* s2, int count)`: function definition. we'll take two pointers and a `count` of bytes to check.
- `        char* c1 = s1;`: here we make a pointer to the provided pointer.
- `        char* c2 = s2;`: here we make a pointer to the provided pointer.
- `        while(count-- > 0)`: here we make a while `count` minus one is higher than zero. `count` will be decremented every time the loop begins again.
- `                if (*c1++ != *c2++)`: here we check if `c1++` and `c2++` are not equal. this increments `*c1` and `*c2` every time the `while` loop runs.
- `                        return c1[-1] < c2[-1] ? -1 : 1;`: and here we make some tertiary operations. if the value of `c1[-1]` is lower than `c2[-1]`, then we return -1, but we return 1 if it's the other way around.
- `        return 0;`: and we return 0 if the comparison is OK!
For a while I wasn't able to understand that what that tertiary operation was doing. But after a couple minutes of reading, I understand.
## 5.5. `pparser.h`
Now we're getting into the interesting stuff. Before reading this section, I recommend reading about [linked lists](https://www.learn-c.org/en/Linked_lists). It's a fundamental data structure and you will need to understand how they work before getting into the section. Nevertheless, you can also read the path parsing notes in the misc section. Let's continue. As always, we won't go over the header guard.
```
struct path_root
{
        int drive_no;
        struct path_part* first;
};

struct path_part
{
        const char* part;
        struct path_part* next;
};

void pathparser_free(struct path_root* root);
struct path_root* pathparser_parse(const char* path, const char* current_directory_path);
```
- `struct path_root`: we create a `path_root`. this will be the root of our filesystem.
- `        int drive_no;`: the `root` also contains data on the drive number.
- `        struct path_part* first;`: and then it links to a `path_part*` struture `first`, which would be files and directories right after `/`. in UNIX, it could be `bin, home, usr`, etc.
- `struct path_part`: here we define the structure for everything else that is not the root.
- `        const char* part;`: here we establish the identifier for that part of the path. it could be, for example, `bin`.
- `        struct path_part* next;`: and here we establish a link to the `next` item within that directory, which is the same type as the one we're defining right now.
- `void pathparser_free(struct path_root* root);`: here we set a `pathparser_free` function, but we'll see it later.
- `struct path_root* pathparser_parse(const char* path, const char* current_directory_path);`: same here. we'll see it later on.
## 5.6. `pparser.c`
Now we're getting into the MEAT and POTATOES! Let's go!

### 5.6.1. `static int pathparser_get_drive_by_path(const char** path)`
this function will get the drive number by using the `path` passed onto it.
```
static int pathparser_get_drive_by_path(const char** path)
{
        if(!pathparser_path_valid_format(*path))
        {
                return -EBADPATH;
        }
        int drive_no = tonumericdigit(*path[0]);

        // add 3 bytes to skip drive number
        *path += 3;
        return drive_no;
}
```
- `static int pathparser_get_drive_by_path(const char** path)`: here we'll take a pointer to a `char` pointer, or a pointer to a string. the `static` means that the function won't be changing any values passed onto it.
- `        if(!pathparser_path_valid_format(*path))`: here we check if the passed value is a valid format. we'll check this function next.
- `                return -EBADPATH;`: if not, return `-EBADPATH` or -4. we'll see this at the end.
- `        int drive_no = tonumericdigit(*path[0]);`: here we convert the first element of the char array into a numeric digit from an ASCII digit into a decimal digit.
- `        *path += 3;`: here we increment the pointer passed by the user by 3.
- `        return drive_no;`: and we return the `drive_no`.
The `path` in this case is something like `0:/bin/bash`. When we do `*path += 3`, we are effectively moving the pointer past the `0:/` section of the path. This isn't changing the pointer, it's just incrementing it, as the data contained has not been changed.
### 5.6.2. `static int pathparser_path_valid_format(const char* filename)`
This function will validate the path format.
```
static int pathparser_path_valid_format(const char* filename)
{
        int len = strnlen(filename, PEACHOS_MAX_PATH);
        return (len >= 3 && isdigit(filename[0]) && memcmp((void*)&filename[1], ":/", 2) == 0);
}
```
- `static int pathparser_path_valid_format(const char* filename)`: here we take a `*filename`, effectively a path. we won't be changing the values passed onto us.
- `        int len = strnlen(filename, PEACHOS_MAX_PATH);`: here we first check if the `len` of `filename` is higher than the maximum path size, which is 108. we'll see this change at the end.
- `        return (len >= 3 && isdigit(filename[0]) && memcmp((void*)&filename[1], ":/", 2) == 0);`: here we make a wacky conditional return. we first check if `len` is higher or equal to 3, we then check if the first character is a digit and then we make a `memcmp` between the second and third item in the `filename` array with `:/`. if all of these conditions are met, the path is valid and we return zero.
### 5.6.3. `static struct path_root* path_parser_create_root(int drive_number)`
This function will setup the root path structure.
```
static struct path_root* path_parser_create_root(int drive_number)
{
        struct path_root* path_r = kzalloc(sizeof(struct path_root));
        path_r->drive_no = drive_number;
        path_r->first = 0;
        return path_r;
}
```
- `static struct path_root* path_parser_create_root(int drive_number)`: here we take the drive number as an argument.
- `        struct path_root* path_r = kzalloc(sizeof(struct path_root));`: now we create a `path_root` structure and we kzalloc
- `        path_r->drive_no = drive_number;`: here we access the member `drive_no` of the pointer to the pointer that points to the `path_root` structure and set the `drive_no` item to `drive_number`.
- `        path_r->first = 0;`: here we do the same as above. `0` in this case represents the first section of our path, which is `0`, like in `0:/bin/bash`.
- `        return path_r;`: and here we return the pointer to the pointer to the struct.
### 5.6.4. `struct path_part* pathparser_parse_path_part(struct path_part* last_part, const char** path)`
This function will parse the path part from the `last_part` using the `path`.
```
struct path_part* pathparser_parse_path_part(struct path_part* last_part, const char** path)
{
        const char* path_part_str = pathparser_get_path_part(path);
        if (!path_part_str)
        {
                return 0;
        }

        struct path_part* part = kzalloc(sizeof(struct path_part));
        part->part = path_part_str;
        part->next = 0x00;

        if (last_part)
        {
                last_part->next = part;
        }

        return part;
}
```
- `struct path_part* pathparser_parse_path_part(struct path_part* last_part, const char** path)`: function definition. we'll use this function to parse the path parts that are passed to us by `last_part` and the pointer to the char pointer `path` itself.
- `        const char* path_part_str = pathparser_get_path_part(path);`: here we get the path by using the `pathparse_get_path_part`, which we'll see next.
- `        if (!path_part_str)`: here we check if the `path_part_str` isn't zero. if it is...
- `                return 0;`: we return zero.
- `        struct path_part* part = kzalloc(sizeof(struct path_part));`: here we allocate memory for our structure with the size of the structure itself.
- `        part->part = path_part_str;`: here we access the `part->part` member of the structure and assign it the `path_part_str` path.
- `        part->next = 0x00;`: here we assign `0x00` as a default value; this is because we don't know if there is a next part.
- `        if (last_part)`: but if there is a next part...
- `                last_part->next = part;`: we assign it to our structure.
- `        return part;`: and we return `part`.
Kinda complex, I know. Read it all the times you need to understand it. Or mail me!
### 5.6.5. `static const char* pathparser_get_path_part(const char** path)`
This function will get the path from the path part.
```
static const char* pathparser_get_path_part(const char** path)
{
        char* result_path_part = kzalloc(PEACHOS_MAX_PATH);
        int i = 0;
        while(**path != '/' && **path != 0x00)
        {
                result_path_part[i] = **path;
                *path += 1;
                i++;
        }
        if (**path == '/')
        {
                // skip / to avoid problems
                *path += 1;
        }

        if (i == 0)
        {
                kfree(result_path_part);
                result_path_part = 0;
        }
        return result_path_part;
}
```
- `static const char* pathparser_get_path_part(const char** path)`: here we take a pointer to a pointer to a char `path`.
- `        char* result_path_part = kzalloc(PEACHOS_MAX_PATH);`: here we allocate `result_path_part` with size 108.
- `        int i = 0;`: here we initialize a counting variable.
- `        while(**path != '/' && **path != 0x00)`: here we check if the element in `path` isn't `/` and if the element isn't `0x00`.
- `                result_path_part[i] = **path;`: while in the loop, we assign `**path` (the element of the pointer to the pointer) to `result_path_part[i]`.
- `                *path += 1;`: now we increment the pointer by one.
- `                i++;`: and we also increment `i` by one, to populate the enxt item in the `result_path_part`, if there is an item, that is.
- `        if (**path == '/')`: after finishing the loop, we check if the element in the `**path` is a `/`. if so...
- `                *path += 1;`: we increment if by one, moving past the `/`
- `        if (i == 0)`: we also check if `i` is zero. if it is, it means that we got the root or that we were already at the end of the path.
- `                kfree(result_path_part);`: and then we free the memory in the `result_path_part` used by the structure.
- `                result_path_part = 0;`: and we set the `result:_path_part` to zero.
- `        return result_path_part;`: and we return!
It's a little bit complex, all of this. I understand if you might feel a bit surpassed by all of this, but don't feel discouraged if you don't understand the code. Go over it, read line by line, use the debugger to go over the code execution, or hit me with an email. I'll find the time to help out :)
### 5.6.6. `struct path_root* pathparser_parse(const char* path, const char* current_directory_path)`
This will be the function that the programmer will see. It's a wrapper that uses all our previous functions. It's the last complex function, just a bit more to be done!
```
struct path_root* pathparser_parse(const char* path, const char* current_directory_path)
{
        int res = 0;
        const char* tmp_path = path;
        struct path_root* path_root = 0;
        if (strlen(path) > PEACHOS_MAX_PATH)
        {
                goto out;
        }

        res = pathparser_get_drive_by_path(&tmp_path);
        if (res < 0)
        {
                goto out;
        }

        path_root = path_parser_create_root(res);
        if(!path_root)
        {
                goto out;
        }
        struct path_part* first_part = pathparser_parse_path_part(NULL, &tmp_path);
        if (!first_part)
        {
                goto out;
        }

        path_root->first = first_part;

        struct path_part* part = pathparser_parse_path_part(first_part, &tmp_path);

        while(part)
        {
                part = pathparser_parse_path_part(part, &tmp_path);
        }

out:
        return path_root;
}
```
- `struct path_root* pathparser_parse(const char* path, const char* current_directory_path)`: function definition. we'll use this function to parse the path passed on to us by the user, and in the future we'll take into consideration the `current_directory_path`, but not right now.
- `        int res = 0;`: here we set `res` for use later on.
- `        const char* tmp_path = path;`: and here we create a pointer to the pointer `path` as to not modify the original pointer.
- `        struct path_root* path_root = 0;`: here we initialize the root_path.
- `        if (strlen(path) > PEACHOS_MAX_PATH)`: here we check if the size of the `char* path` passed onto us fits in the 108 byte limit we impossed. if `path` is higher than 108...
- `                goto out;`: we go to out and return 0.
- `        res = pathparser_get_drive_by_path(&tmp_path);`: here we get the drive number. it will also set `res` to the `drive_no`, which we'll use later.
- `        if (res < 0)`: here we check if `res` is lower than zero. if it is...
- `                goto out;`: we go to the `out` label and return 0.
- `        path_root = path_parser_create_root(res);`: here is where we modfiy the actual `path_root` structure, passing in our `drive_no`. this will return `path_r`, which is a `path_root*` structure with `root` values set; that is, `drive_no` is zero and `first` is `0`.
- `        if(!path_root)`: here we check if `path_root` isn't set, and if it isn't...
- `                goto out;`: we go to the `out` label and return 0.
- `        struct path_part* first_part = pathparser_parse_path_part(NULL, &tmp_path);`: here we initialize the `first_part` part of our path. we pass `NULL` because there is no previous part of the path yet. we also pass the `&tmb_path` (value of `tmp_path`) to the function. this function will return a `path_part` structure with populated values of the first item in the path and `0x00` as the `next` value, since there is no next part of the path yet.
- `        if (!first_part)`: here we check if `first_part` isn't set. if it isn't...
- `                goto out;`: we go to the `out` label and return 0.
- `        path_root->first = first_part;`: now we can link our `path_root` with its `first` part, which is the `first_part`, a `path_part` structure that contains it's own value and the next one.
- `        struct path_part* part = pathparser_parse_path_part(first_part, &tmp_path);`: now we'll initialize the next value. it'll we stored in the `part` pointer.
- `        while(part)`: and if `part` is set...
- `                part = pathparser_parse_path_part(part, &tmp_path);`: we can recursively assign each `next` part of our linked list.
- `out:`: `out` label.
- `        return path_root;`: and finally we return the linked list. the `path_root` contains all the items from `part` and whatever they might link to.
That was the hardest part, the combination of everything we've written so far. Congratulations if you understood everything! It took me a couple hours to fully get, but I did it!
### 5.6.7. `void pathparser_free(struct path_root* root)`
And this function will free the allocated spaces of heap that we used during the root creation and path parsing.
```
void pathparser_free(struct path_root* root)
{
        struct path_part* part = root->first;
        while(part)
        {
                struct path_part* next_part = part->next;
                kfree((void*)part->part);
                kfree(part);
                part = next_part;
        }
        kfree(root);
}
```
- `void pathparser_free(struct path_root* root)`: function definition. we'll just take the `path_root` object, since it already contains all the values we need from the linked list after initializing it.
- `        struct path_part* part = root->first;`: here we access the `path_part` via our `path_root` `root` object.
- `        while(part)`: and now we get into a loop that will run until `part` isn't set.
- `                struct path_part* next_part = part->next;`: here we **must** assign the `next_part` before freeing the memory.
- `                kfree((void*)part->part);`: now we free the `part` section of our path `part`
- `                kfree(part);`: and now we can free the structure in its entirety.
- `                part = next_part;`: and now we reassign the `part` pointer to the `next_part` pointer that we created previously.
- `        kfree(root);`: after freeing every other member of the linked list, we can free the `path_root` `root` pointer.
And done! The `pathparser_free` is way simpler than everything else we've seen so far, but it's an important function to make. Now, we'll continue to see the other minor changes we made to `kernel.h` and `status.h`.
## 5.7. `kernel.h`
We've just added some constants. Nothing complex.
```
...
#define PEACHOS_MAX_PATH 108
...
```
- `#define PEACHOS_MAX_PATH 108`: This constant will determine the maximum lenght of a part of a path.
## 5.8 `status.h`
We also added a new status to the file, which is just a constant.
```
...
#define EBADPATH 4
...
```
- `#define EBADPATH 4`: This status code will be used for bad paths, as we saw below.

## 5.9. `kernel.c`
This is the least important part. I'll show you how we can use the parser, just for demonstration purposes.
```
        struct path_root* root_path = pathparser_parse("0:/usr/bin/bash", NULL);

        if(root_path)
        {
                print("[+] pathparse yay!");
        }
```
- `        struct path_root* root_path = pathparser_parse("0:/usr/bin/bash", NULL);`: here we use the wrapper, passing a path (whatever that might be) and `NULL`, since we don't a current working directory.
- `        if(root_path)`: unnecesary, but it's fun to get print messages when stuff works.
- `                print("[+] pathparse yay!");`: and print some message.

If we run the kernel, we won't see much happening apart from the `print` message. To check the results, we need to `print root_path` in GDB.
![gdb root_path](https://github.com/4nt11/theos/blob/main/media/pparser.jpg)

# 6. Creating a disk stream.
We will implement a way in which we can stop reading from the disk in sectors and start reading from it in bytes.
## 6.1. `streamer.h`
We've created a header file for our streamer. We'll just define some structures and prototypes.
```
#ifndef DISKSTREAMER_H
#define DISKSTREAMER_H
#include "disk.h"

struct disk_stream
{
        int pos;
        struct disk* disk;
};

struct disk_stream* diskstreamer_new(int disk_id);
int diskstreamer_seek(struct disk_stream* stream, int pos);
int diskstreamer_read(struct disk_stream* stream, void* out, int total);
void diskstreamer_close(struct disk_stream* stream);


#endif
```
- `struct disk_stream`: `disk_stream` structure definition.
- `        int pos;`: the `disk_stream` will have a `pos` value that will be used to know where the streamer is currently located.
- `        struct disk* disk;`: and it also has a `disk` `disk` structure, which will point to a given disk.
- `struct disk_stream* diskstreamer_new(int disk_id);`: prototype. we'll see it later.
- `int diskstreamer_seek(struct disk_stream* stream, int pos);`: prototype. we'll see it later.
- `int diskstreamer_read(struct disk_stream* stream, void* out, int total);`: prototype. we'll see it later.
- `void diskstreamer_close(struct disk_stream* stream);`: prototype. we'll see it later.
Now, let's go over the code.
## 6.2. `streamer.c`

### 6.2.1. `struct disk_stream* diskstreamer_new(int disk_id);`
We will use this function to create a streamer.
```
struct disk_stream* diskstreamer_new(int disk_id)
{
        struct disk* disk = disk_get(disk_id);
        if(!disk)
        {
                return 0;
        }

        struct disk_stream* streamer = kzalloc(sizeof(struct disk_stream));
        streamer->pos = 0;
        streamer->disk = disk;

        return streamer;
}
```
- `struct disk_stream* diskstreamer_new(int disk_id)`: function definition. we'll receive a disk number (usually 0) as to point the new `disk_stream` to that disk number.
- `        struct disk* disk = disk_get(disk_id);`: here we get the disk. now it only returns `0` to non `0` values. if zero, it returns a disk object defined in the `disk.c` file.
- `        if(!disk)`: here we check if we got the correct values from `disk_get` call.
- `                return 0;`: if we didn't, return 0.
- `        struct disk_stream* streamer = kzalloc(sizeof(struct disk_stream));`: here we allocate enough heap space for our `streamer` structure.
- `        streamer->pos = 0;`: here we set the initial position of the streamer.
- `        streamer->disk = disk;`: and here we set the disk to the disk passed onto us.
- `        return streamer;`: and we return the streamer object.
### 6.2.2. `int diskstreamer_seek(struct disk_stream* stream, int pos);`
We will use this function to change the `pos` value of the streamer. We'll see the usage of `pos` in the next function.
```
int diskstreamer_seek(struct disk_stream* stream, int pos)
{
        stream->pos = pos;
        return 0;
}
```
- `int diskstreamer_seek(struct disk_stream* stream, int pos)`: function definition. it takes a (previously created) streamer and it's new `pos` value.
- `        stream->pos = pos;`: accessing the `pos` member and setting it to `pos` passed by the user.
- `        return 0;`: and return.
### 6.2.3. `int diskstreamer_read(struct disk_stream* stream, void* out, int total);`
This function will be used to read the data stream into the `out` pointer. We're supposed to pass it a `disk_stream`, the `out` pointer and the `total` amount of bytes. Here, we'll use the `stream->pos` value to read from `pos`.
```
int diskstreamer_read(struct disk_stream* stream, void* out, int total)
{
        int sector = stream->pos / PEACHOS_SECTOR_SIZE;
        int offset = stream->pos % PEACHOS_SECTOR_SIZE;
        char buf[PEACHOS_SECTOR_SIZE];

        int res = disk_read_block(stream->disk, sector, 1, buf);
        if(res < 0)
        {
                goto out;
        }

        int total_to_read = total > PEACHOS_SECTOR_SIZE ? PEACHOS_SECTOR_SIZE : total;
        for(int i = 0; i < total_to_read; i++)
        {
                *(char*)out++ = buf[offset+i];
        }
        stream->pos += total_to_read;
        if(total > PEACHOS_SECTOR_SIZE)
        {
                res = diskstreamer_read(stream, out, total-PEACHOS_SECTOR_SIZE);
        }

out:
        return res;

}
```
- `int diskstreamer_read(struct disk_stream* stream, void* out, int total)`: function definition we'll take a streamer object, a pointer and a `total` amount of bytes to read into the `out` pointer.
- `        int sector = stream->pos / PEACHOS_SECTOR_SIZE;`: here we get the sector of the disk. if the value is, for example, `30`, then `30 / PEACHOS_SECTOR_SIZE` returns 0; if 513, then it's `513 / PEACHOS_SECTOR_SIZE` or 1.
- `        int offset = stream->pos % PEACHOS_SECTOR_SIZE;`: now we get the offset of the bytes in relation to the sector. this will be used as our starting byte to read from. if `pos` is 30, then `30 % PEACHOS_SECTOR_SIZE` equals 30. and if `pos` 513, then `1`.
	- in the second case it returns 1 because it's indeed the first byte of sector 1 (or second sector, counting from zero).
- `        char buf[PEACHOS_SECTOR_SIZE];`: here we assign a buffer of `PEACHOS_SECTOR_SIZE` bytes to read into. we don't care if we're reading less than that, we need a buffer big enough as to not cause overflows.
- `        int res = disk_read_block(stream->disk, sector, 1, buf);`: here we use the `disk_read_block` that we previously made. this will only read the `sector` calculated previously and store the data into `buf`.
- `        if(res < 0)`: here we check if the read was successful.
- `                goto out;`: if not, return the error.
- `        int total_to_read = total > PEACHOS_SECTOR_SIZE ? PEACHOS_SECTOR_SIZE : total;`: this is a ternary assignation we first check if the `total` bytes to read is higher than the `PEACHOS_SECTOR_SIZE` and if so, assign the `PEACHOS_SECTOR_SIZE` to `total_to_read`. if lower, assign the total value.
	- this is a recursive function. if we were to read 514 bytes, the function itself would run within itself. we'll see this in a bit.
- `        for(int i = 0; i < total_to_read; i++)`: `for` loop to read the bytes.
- `                *(char*)out++ = buf[offset+i];`: tricky pointer magic. here, we cast the `out` pointer into a `char` pointer and then dereferencing the pointer. after that, we assign the `buf[offset+i]` to the current pointer position and finally incrementing it. we assign `buf[offset+i]` because `offset+i` it's the byte that was asked from the function.
	- `out++` and `++out` are different. `out++` increments after assignment and `++out` increments prior to assignment.
- `        stream->pos += total_to_read;`: here we increment the `pos` by `total_to_read` bytes. this is because we've already read those bytes.
- `        if(total > PEACHOS_SECTOR_SIZE)`: here we check again if `total` is higher than `PEACHOS_SECTOR_SIZE`. if so...
- `                res = diskstreamer_read(stream, out, total-PEACHOS_SECTOR_SIZE);`: we call ourselves, but `total` is now subtracted `PEACHOS_TOTAL_SECTORS`. this changes the `total` variable for the next pass of the function, and will do until it becomes 0. this new call also modifies the `out` pointer directly (as we previously did), so there's no need to return `out`.
- `out:`: `out` label.
- `        return res;`: here we return `res`.
That was a little bit of pointer magic. But we did it.
### 6.2.4. `void diskstreamer_close(struct disk_stream* stream);`
And when we're done using the stream, we can call this function to free the memory that it used.
```
void diskstreamer_close(struct disk_stream* stream)
{
        kfree(stream);
}
```
- `void diskstreamer_close(struct disk_stream* stream)`: function definition. it takes a previously created `streamer` object.
- `        kfree(stream);`: here we free the `stream` pointer.
# 7. File Allocation Table (FAT)
- It's a filesystem developed by Microsoft.
- It consists of a series of clusters of data and a table that determines the state of the clusters.
- The boot sector contains information about the filesystem.
## 7.1. FAT16 Filesystem
- This filesystem uses clusters to represent data, directories and files.
- Each and every cluster uses a fixed amount of sectors which is specified in the boot sector.
- Every file in tha FAT16 filesystem needs to use at least one cluster for its data. This means that a lot of storage is wasted for small files.
- FAT16 can't store files larger than 2 gigabytes without large file support. With large file support, it can hold files up to 4 gigabytes.
- It's very easy to implement.
## 7.2. FAT16 Disk Layout

| Name              | Size                                                                                     |
| ----------------- | ---------------------------------------------------------------------------------------- |
| Boot sector       | 512 bytes.                                                                               |
| Reserved sectors. | fat_header.reserved_sectors \* 512*                                                      |
| FAT 1             | fat_header.sectors_per_fat \* 512                                                        |
| FAT 2 (Optional)  | fat_header.sectors_per_fat \* 512                                                        |
| Root directory.   | fat_header.root_dir_entries \* sizeof(struct fat_directory_item) (**rounded if needed**) |
| Data clusters.    | ...rest of disk.                                                                         |
## 7.3. FAT16 Boot Sector
The FAT16 boot sector is basically the Boot Parameter Block. Check the BIOS BPB entry on the misc notes.
## 7.4. FAT16 File Allocation: Clusters in disk.
- In the FAT table, we'll find that each cluster is represented as an entry of two bytes. Each two byte represent a cluster in the data region that is taken, free or in use.
- Clusters can be chained together. An example would be a file larger than a cluster. It'd use one or more clusters, obviously.
- When files use more than one cluster, the first cluster entry will too point to the next cluster in use and so on. The final cluster will contain the value `0xFFFF`, which means the end of the cluster chain.
- The size of a cluster is determined by the values in the boot sector
Example cluster table:

| Entry zero | Entry one  | Entry two | Entry three |
| ---------- | ---------- | --------- | ----------- |
| 0x0003     | 0xFFFF     | 0xFFFF    | 0xFFFF      |
| Entry four | Entry five | Entry six | Entry seven |
| 0x0000     | 0x0000     | 0x0000    | 0x0000      |
We have a cluster chain at entry zero. We know this because the first cluster entry has decimal 3. So, counting from zero, we can see that the final cluster would be located at entry three.
\*Note: Cluster zero isn't used most of the time. FAT usually starts from cluster three and so forth.
## 7.5. How are clusters accessed?
If we wanted to, for example, access the third cluster in the FAT table, we'd take the

`cluster index * size of cluster`

If each cluster is 512 bytes in size and we wanted the data from the third cluster, we'd do: `3 * 512` and we'd have to access bytes 1536.
## 7.6. FAT16 Root Directory
- It's the top directory, like `C:\` or `/`.
- Directories contain directory entries of a fixed size.
## 7.7. FAT16 Directory Entry
A typical directory entry would look like this:
```
struct fat_directory_item
{
	uint8_t filename[8];
	uint8_t ext[3].
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
```
It's a lot of stuff, but everything is quite simple. FYI: the `attributes` item is used to declare the type of file (file or directory), read access, write access and so on.
## 7.8 Iterating through directories
- The boot sector contains the maximum number of root directory entries. We should not exceed this value when iterating through the root directory.
- We know that when we finish iterating through the root directory or a subdirectory, because the first byte of our filename will be equal to zero.
## 7.9. Directory entry attribute flags

| Flag   | Description                        |
| ------ | ---------------------------------- |
| `0x01` | Read only.                         |
| `0x02` | File hidden.                       |
| `0x04` | System file. Do not move clusters. |
| `0x08` | Volume label.                      |
| `0x10` | This is a directory.               |
| `0x20` | Archived.                          |
| `0x40` | Device.                            |
| `0x80` | Reserved.                          |
## 7.10. Filenames and extensions.
- The filename is 8 bytes wide and unused bytes are padded with spaces (`0x20`).
- The extension is 3 bytes wide and unused bytes are padded with spaces (`0x20`).
## 7.11. Clusters
- Each cluster represents a fixed amount of sectors in the disk, linearly to each other.
- The amount of sectors that represents a cluster is stored in the boot sector.
- The data clusters section in the filesystem contains all the clusters that make up the subdirectories and file data of files throughout the FAT filesystem.
## 7.12. Tips
- Always use `__attribute__((packed))` when working with structures that are to be stored or read from the disk. The C compiler might try to optimize the structure and change it. We **DO NOT** want this when working with raw data to or from and the disk.
- Learn to use GDB!
# 8. Starting our FAT Filesystem.
To begin creating our FAT filesystem, we first need to modify the bootloader (`boot.asm`).
## 8.1. `boot.asm`
The added code is just added variables in Assembly.  ~~Not actually variables, but you understand~~- We did make an important change, which is deleting the `times db 33` we had before. We don't need that anymore, because we'll implement a proper BPB; which is also our FAT table.
```
ORG 0x7c00
BITS 16

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

jmp short start
nop

; FAT16 Header
OEMIdentifier           db 'PEACHOS '
BytesPerSector          dw 0x200
SectorsPerCluster       db 0x80
ReservedSectors         dw 200
FATCopies               db 0x02
RootDirEntries          dw 0x40
NumSectors              dw 0x00
MediaType               db 0xF8
SectorsPerFAT           dw 0x100
SectorsPerTrack         dw 0x20
NumberOfHeads           dw 0x40
HiddenSectors           dd 0x00
SectorsBig              dd 0x773544

; Entended BPB
DriveNumber             db 0x80
WinNTBit                db 0x00
Signature               db 0x29
VolumeID                dd 0xD105
VolumeIDString          db 'PEACHOS BOO'
SystemIDString          db 'FAT16   '
```
- `jmp short start`: start of the BPB. we immediately make a `jump short` into the `start` label. the BPB isn't meant to be executed.
- `nop`: as per BPB, we require a `nop` instruction here.
- `OEMIdentifier           db 'PEACHOS '`: this is the OEM Identifier. we can put whatever we want here. there are some old DOS drivers that only work with some OEM ids, but those are far gone. this is an 8 byte value padded with spaces. that is, if we use less that 8 bytes, we must add the spaces.
- `BytesPerSector          dw 0x200`: these is the amount of bytes per *logical* (not physical) sectors. I emphasize *logical* because this parameter doesn't change the physical sectors byte amount, only the logical. that is, what the OS sees. `0x200` is 512 bytes.
- `SectorsPerCluster       db 0x80`: amount of sectors per cluster. we use one of the allowed values `0x80` or 128 sectors per cluster.
- `ReservedSectors         dw 200`: these are reserved sectors, i.e., sectors to be ignored by the kernel. keep in mind that these sectors must be manually ignored.
- `FATCopies               db 0x02`: number of FAT table copies.
- `RootDirEntries          dw 0x40`: number of maximum root directory entries in the FAT12 or FAT16 filesystem. this value must be divisible by 16. in our case, we use `0x40` or 64, which is indeed divisible by 16.
- `NumSectors              dw 0x00`: logical sectors. we won't use this for now.
- `MediaType               db 0xF8`: this is the media descriptor. `0xF8` describes a partition.
- `SectorsPerFAT           dw 0x100`: these are the amount of sectors per FAT. we'll use `0x100` or 256.
- `SectorsPerTrack         dw 0x20`: these are the sectors per track. 
- `NumberOfHeads           dw 0x40`: and these are the number of heads. it's related to CHS geometry, as the entry above.
- `HiddenSectors           dd 0x00`: these are the hidden sectors. literally just that.
- `SectorsBig              dd 0x773544`: since we aren't using the `NumSectors`, we must set `SectorsBig`, which is the total amount of sectors in the disk.
- `; Entended BPB`: separator. from now on, we'll see the EBPB or Extended BIOS Parameter Block.
- `DriveNumber             db 0x80`: this is the drive number. `0x80` means "first physical disk".
- `WinNTBit                db 0x00`: this byte is resereved.
- `Signature               db 0x29`: this is the signature. it must be `0x29`.
- `VolumeID                dd 0xD105`: this is the serial number of the drive.
- `VolumeIDString          db 'PEACHOS BOO'`: this is the name or label of the drive. it must be 11 bytes, and if less than 11 bytes are used, you need to pad the rest with spaces.
- `SystemIDString          db 'FAT16   '`:  this is the file system type. this is an 8 byte value and it must be padded with spaces.

And that's that! 
# 8.2. `Makefile`
We also modified the `Makefile` a little bit.
```
all: ./bin/kernel.bin ./bin/boot.bin
        ...
        dd if=/dev/zero bs=1048576 count=16 >> ./bin/os.bin
```
- `dd if=/dev/zero bs=1048576 count=16 >> ./bin/os.bin`: we changed the block size to 1048576 (1 megabyte) and the count to 16.
# 9. Understanding VFS or Virtual File System.
- The VFS layer allows a kernel to support an infinite amount of filesystems.
- It allows us to abstract complicated low level functions with higher, simpler interfaces.
- It also allows the kernel to load and unload filesystem functionality at will.
- The VFS layer is supposed to be used by all other filesystems.
## 9.1. What happens when we insert a disk?
- The kernel checks for its filesystems and then asks the drive if it has a filesystem it can handle. This process is called resolving the filesystem.
- If the kernel has functionality for that filesystem, it will then load that functionality and binds it to itself.
- Userspace programs use syscalls to the kernel to interact with the drives. There is no direct interaction between the user and the hardware.
Take the following example:
```
-> Userspace executes fopen("0:/test.txt", "r")
-> The kernel then parses the call
-> Path parser returns path root.
-> disk_read then talks to the drive
-> the drive calls the `fopen` FAT32 function and returns a file descriptor.
```
# 10. Implementing the VFS Core Functionality.
## 10.1. `config.h`
We defined some magic numbers for convenience an readability.
```
// max fs
#define PEACHOS_MAX_FILESYSTEMS         12

// max open files
#define PEACHOS_MAX_FILEDESCRIPTORS     512
```
- `#define PEACHOS_MAX_FILESYSTEMS         12`: we've added a max filesystem number. it's completely arbitrary and can be anything we want.
- `#define PEACHOS_MAX_FILEDESCRIPTORS     51`: and a maximum amount of open files in our system. as before, this is arbitrary and can be whatever we want.
## 10.2. `file.h`
sometext
```
#ifndef FILE_H
#define FILE_H
#include "pparser.h"

typedef unsigned int FILE_SEEK_MODE;
enum
{
        SEEK_SET,
        SEEK_CUR,
        SEEK_END,
};

typedef unsigned int  FILE_MODE;
enum
{
        FILE_MODE_READ,
        FILE_MODE_WRITE,
        FILE_MODE_APPEND,
        FILE_MODE_INVALID
};

struct disk;
typedef void*(*FS_OPEN_FUNCTION)(struct disk* disk, struct path_part* path, FILE_MODE mode);
typedef int (*FS_RESOLVE_FUNCTION)(struct disk* disk);

struct filesystem
{
        // fs should return 0 from resolve if the disk is using its fs.
        FS_RESOLVE_FUNCTION resolve;
        FS_OPEN_FUNCTION fopen;
        char name[20];
};

struct file_descriptor
{
        int index;
        struct filesystem* filesystem;
        // private data for internal fd
        void* private;
        // disk the fd is used on
        struct disk* disk;
};

void fs_init();
int fopen(const char* filename, const char* mode);
void fs_insert_filesystem(struct filesystem* filesystem);
struct filesystem* fs_resolve(struct disk* disk);

#endif
```
- `#include "pparser.h"`: we include `pparser.h` to use it later.
- `typedef unsigned int FILE_SEEK_MODE;`: here we define a type `FILE_SEEK_MODE`. it will represent the start of the file, the current relative position or the end of the file.
- `enum`: and with the `enum` keyword, we define the values for `FILE_SEEK_MODE`.
- `        SEEK_SET,`: for the start of the file,
- `        SEEK_CUR,`: for the current relative position and
- `        SEEK_END,`: for the end of the file.
- `typedef unsigned int  FILE_MODE;`: now we define the different file modes in which a file descriptor can be opened with.
- `enum`: again, we define the values for `FILE_MODE`.
- `        FILE_MODE_READ,`: for read mode,
- `        FILE_MODE_WRITE,`: for write mode,
- `        FILE_MODE_APPEND,`: for append (add characters after EOF) and
- `        FILE_MODE_INVALID`: for any invalid mode.
- `struct disk;`: here we define a `disk` struct which is globally accessible.
- `typedef void*(*FS_OPEN_FUNCTION)(struct disk* disk, struct path_part* path, FILE_MODE mode);`: this is a function pointer that takes a `disk`, a `path_part` pointer to a file and a `file_mode`.
	- a function pointer is used to point to a function that will be defined by the programmer later on. check the Function Pointers misc page for more.
- `typedef int (*FS_RESOLVE_FUNCTION)(struct disk* disk);`: another function pointer that will take a `disk` function. this resolve function is used to resolve the filesystem implementation as per the filesystem check.
- `struct filesystem`: here we define a `filesystem` struct, which will be used by all the fs drivers that will use this VFS interface to interact with files.
- `        FS_RESOLVE_FUNCTION resolve;`: the struct must contain a function to resolve the filesystem and be able (or not) to recognize the filesystem as theirs.
- `        FS_OPEN_FUNCTION fopen;`: it'll also have a `fopen` function defined by the driver itself.
- `        char name[20];`: and a name of the filesystem, like FAT or EXT.
- `struct file_descriptor`: now we define the `file_descriptor` struct. all the files in our OS will fall under this struct. each filesystem driver will use this struct to return the data read from the filesystem.
- `        int index;`: here we have an index, or a position in the disk in which this file is in.
- `        struct filesystem* filesystem;`: the `filesystem` the file resides on,
- `        void* private;`: a pointer to private data, if any.
- `        struct disk* disk;`: and a `disk` struct in which the file is in.
- `void fs_init();`: function prototype. we'll see this later.
- `int fopen(const char* filename, const char* mode);`: function prototype. we'll see this later.
- `void fs_insert_filesystem(struct filesystem* filesystem);`: function prototype. we'll see this later.
- `struct filesystem* fs_resolve(struct disk* disk);`: function prototype. we'll see this later.
This is the basis of our VFS layer. As mentioned before, it'll we used by all other filesystems to interface with the OS.
## 10.3. `file.c`
Here's the base implementation. There's still a lot to be done, but we'll get there when we get there.
```
#include "file.h"
#include "status.h"
#include "kernel.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "config.h"

struct filesystem* filesystems[PEACHOS_MAX_FILESYSTEMS];
struct file_descriptor* file_descriptors[PEACHOS_MAX_FILEDESCRIPTORS];

static struct filesystem** fs_get_free_filesystem()
{
        int i = 0;
        for (i = 0; i < PEACHOS_MAX_FILESYSTEMS; i++)
        {
                return &filesystems[i];
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
        //fs_insert_filesystem(fat16_init());
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

int fopen(const char* filename, const char* mode)
{
        return -EIO;
}
```
We will the read the code as it would execute in memory. It's a bit much, but not complex. First, we'll go over the initialization code. Then, we'll go over the others.
### 10.3.0. Initial definitions.
Before jumping into each function, we need to see the initial structs that are defined by the file.
```
struct filesystem* filesystems[PEACHOS_MAX_FILESYSTEMS];
struct file_descriptor* file_descriptors[PEACHOS_MAX_FILEDESCRIPTORS];
```
- `struct filesystem* filesystems[PEACHOS_MAX_FILESYSTEMS];`: here we define the general `filesystems` structure. it's configured to be of `PEACHOS_MAX_FILESYSTEMS` or 12.
- `struct file_descriptor* file_descriptors[PEACHOS_MAX_FILEDESCRIPTORS];`: and the general `file_descriptors` structure, which is `PEACHOS_MAX_FILEDESCRIPTORS` or 512.
### 10.3.1. void fs_init()
```
void fs_init()
{
        memset(file_descriptors, 0, sizeof(file_descriptors));
        fs_load();
}
```
- `void fs_init()`: `fs_init` function. it'll setup the filesystems as it executes; initially, it cleans up `file_descriptors`.
- `        memset(file_descriptors, 0, sizeof(file_descriptors));`: here we `memset` the initial `file_descriptors` to zero.
- `        fs_load();`: and we call `fs_load`.
### 10.3.2. void fs_load()
```
void fs_load()
{
        memset(filesystems, 0, sizeof(filesystems));
        fs_static_load();
}
```
- `void fs_load()`: `fs_load` function. here we `memset` the `filesystems` structure and continue executing.
- `        memset(filesystems, 0, sizeof(filesystems));`: `memset` call to setup `filesystems` to 0.
- `        fs_static_load();`: and we now call `fs_static_load`.
### 10.3.3. static void fs_static_load()
```
static void fs_static_load()
{
        //fs_insert_filesystem(fat16_init());
}
```
- `static void fs_static_load()`: function definition. for now, it doesn't do much.
- `        //fs_insert_filesystem(fat16_init());`: we call `fs_insert_filesystem` with `fat16_init` function, which would return a `filesystem` structure. the function hasn't been implemented yet, to its commented out for now.
### 10.3.4. void fs_insert_filesystem(struct filesystem *filesystem)
```
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
```
- `void fs_insert_filesystem(struct filesystem *filesystem)`: function definition. it'll take a `filesystem` structure and then insert them into the `filesystems` structure.
- `        struct filesystem** fs;`: here we define a pointer to a pointer to a `filesystem` structure.
- `        fs = fs_get_free_filesystem();`: here we get the value address returned by the `fs_get_free_filesystem`. we'll see it later.
- `        if(!fs)`: here we check if the `fs` pointer is null. if so...
- `                print("no fs!");`: we panic!
- `                while(1);`: but we don't have a `panic` function yet. so the system just enters an unescapable `while` loop.
- `        *fs = filesystem;`: if not panicking, we dereference the `*fs` pointers (which in turn accesses the `filesystems` structure defined at the beginning of the file) and sets its value to `filesystem`.
### 10.3.5. static struct filesystem** fs_get_free_filesystem()
```
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
```
- `static struct filesystem** fs_get_free_filesystem()`: this function will return the memory address value of the first NULL pointer.
- `        int i = 0;`: initial counter.
- `        for (i = 0; i < PEACHOS_MAX_FILESYSTEMS; i++)`: `for` loop that will run 12 times.
- `               if(filesystems[i] == 0)`: here we check if the `filesystems[i]` is NULL. if so...
- `                       return &filesystems[i];`: we access the memory value of the `filesystems[i]` and return it to the caller, which is the function we saw before.
- `        return 0;`: and if anything fails, we return 0.
And that's it! Pointers get a little bit tricky, but you'll get the hang of it. Now, we need to read the helper functions.
### 10.3.6. `static int file_new_descriptor(struct file_descriptor** desc_out)`
```
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
```
- `static int file_new_descriptor(struct file_descriptor** desc_out)`: this function will be used to create file descriptors.
- `        int res = -ENOMEM;`: here we create the initial `res` value.
- `        for (int i = 0; i < PEACHOS_MAX_FILEDESCRIPTORS; i++)`: here we start enumerating all the file descriptors.
- `                if (file_descriptors[i] == 0)`: here we check if `file_descriptors[i]` is NULL or unset.
- `                        struct file_descriptor* desc = kzalloc(sizeof(struct file_descriptor));`: here we create the initial structure that will be assigned to the file descriptors, allocate memory and zero it out.
- `                        desc->index = i+1;`: here we increase the `index` value, which would become 1.
- `                        file_descriptors[i] = desc;`: and we assign the value of `desc` to the found NULL descriptor.
- `                        *desc_out = desc;`: and we modify the descriptor passed to us to point to the newly created file descriptor.
- `                        res = 0;`: and we set `res` to 0.
- `                        break;`: and we break the loop.
- `        return res;`: and return `res`.
### 10.3.7. `static struct file_descriptor* file_get_descriptor(int fd)`
```
static struct file_descriptor* file_get_descriptor(int fd)
{
        if(fd <= 0 || fd >= PEACHOS_MAX_FILEDESCRIPTORS)
        {
                return 0;
        }
        // descriptors start at 1, but arrays values start at 0.
        int index = fd - 1;
        return file_descriptors[index];
}
```
- `static struct file_descriptor* file_get_descriptor(int fd)`: this function will the return a file descriptor.
- `        if(fd <= 0 || fd >= PEACHOS_MAX_FILEDESCRIPTORS)`: here we check if the `fd` value passed to us is zero or is a value higher than the allowed file descriptors.
- `                return 0;`: and we return zero if any of the conditions are met.
- `        int index = fd - 1;`: here we create an index value to access the `fd`. if `fd` is one, then we need to access the previous value (`0`) to access the `fd` 1.
- `        return file_descriptors[index];`: and we return the file descriptor.
### 10.3.8. `struct filesystem* fs_resolve(struct disk* disk)`
```
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
```
- `struct filesystem* fs_resolve(struct disk* disk)`: this function will use the `resolve` function pointer to get the fs type.
- `        struct filesystem* fs = 0;`: here we make a `fs` pointer to `filesystem` structure to zero (null).
- `        for (int i = 0; i < PEACHOS_MAX_FILESYSTEMS; i++)`: we start a loop that will run at most 12 times.
- `                if (filesystems[i] != 0 && filesystems[i]->resolve(disk) == 0)`: here we make two checks: first we check if the `filesystems[i]` is NULL and if the execution of the `resolve` function pointer is zero. if so...
- `                        fs = filesystems[i];`: we set `fs` to `filesystems[i]`.
- `                        break;`: and break.
- `        return fs;`: and we return the `fs` to the caller.
### 10.3.9. `int fopen(const char* filename, const char* mode)`
```
int fopen(const char* filename, const char* mode)
{
        return -EIO;
}
```
- `int fopen(const char* filename, const char* mode)`: we create the skeleton for our `fopen` function.
- `        return -EIO;`: we haven't implemented it yet, so we return `-EIO`.
## 10.4. `disk.h`
sometext
```
...
struct disk
{
        PEACHOS_DISK_TYPE type;
        int sector_size;
        struct filesystem* filesystem;
};
...
```
- `struct disk`:
- ...
- `        struct filesystem* filesystem;`: we added the `filessytem` structure to our `disk` structure, as a way to identify the disk.
## 10.5. `disk.c`
sometext
```
void disk_search_and_init()
{
        memset(&disk, 0, sizeof(disk));
        disk.type = PEACHOS_DISK_TYPE_REAL;
        disk.sector_size = PEACHOS_SECTOR_SIZE;
        disk.filesystem = fs_resolve(&disk);
}
```
- `void disk_search_and_init()`:
- `        disk.filesystem = fs_resolve(&disk);`: here we make a call to the function `fs_resolve`, which in turn will call the `filesystem->FS_RESOLVE_FUNCTION` function pointer defined in the `file.h` header file
## 10.6. `Makefile`
sometext
```
FILES = ... ./build/fs/file.o

all: ./bin/kernel.bin ./bin/boot.bin
	...
	sudo mount -t vfat ./bin/os.bin ./bin/mountpoint
        echo "hello world!" | sudo tee ./bin/mountpoint/helloworld.txt
        sudo umount ./bin/mountpoint

./build/fs/file.o: ./src/fs/file.c
        i686-elf-gcc $(INCLUDES) -I./src/file/ $(FLAGS) -std=gnu99 -c ./src/fs/file.c -o ./build/fs/file.o
```

- ` 	sudo mount -t vfat ./bin/os.bin ./bin/mountpoint`: here, we mount the `os.bin` file into the `mountpoint`. we can do this because Linux recognizes the `os.bin` as a valid FAT16 partition.
- `       echo "hello world!" | sudo tee ./bin/mountpoint/helloworld.txt`: here we write something to the disk. it doesn't matter what it is, we just want to have placeholder data for us to read later on.
- `       sudo umount ./bin/mountpoint`: and here we unmount the disk.
This is definitely not needed, but it'll help us to later on check if our implementations are working or not.
# 11. Implementing FAT16 core functions.
Well... we begin implementing our FAT16 driver. We have a lot of changes to cover. So, let's begin.
## 11.1. `string.c`
First we need a `strcpy` function. It's quite easy to make. Let's see it.
```
char* strcpy(char* dest, const char* src)
{
        char *res = dest;
        while(*src != 0)
        {
                *dest = *src;
                src += 1;
                dest += 1;
        }
        *dest = 0x00;
        return res;
}
```
- `char* strcpy(char* dest, const char* src)`: function definition. we'll take a `dest` pointer and some text.
- `        char *res = dest;`: here we make a pointer to the `dest` pointer.
- `        while(*src != 0)`: `while` loop until `*src` is 0.
- `                *dest = *src;`: we point the `dest` value to the `src` value.
- `                src += 1;`: and we increment the `src` pointer.
- `                dest += 1;`: same with `dest`.
- `        *dest = 0x00;`: after finishing the loop, we must add a null byte. we do this because the `while` loop stops right at the NULL byte.
- `        return res;`: and we return the `res` pointer. we return `res` instead of `dest` because `res` points to `dest`.
## 11.2. `string.h`
```
...
char* strcpy(char* dest, const char* src);
...
```
- `char* strcpy(char* dest, const char* src);`: this is just a prototype of the function we saw before.
## 11.3. `file.c`
```
...
static void fs_static_load()
{
        fs_insert_filesystem(fat16_init());
}
...
```
- `        fs_insert_filesystem(fat16_init());`: here we just uncomment this line.
## 11.4. `fat16.h`
We begin!
```
#ifndef FAT16_H
#define FAT16_H
#include "fs/file.h"

struct filesystem* fat16_init();

#endif
```
- `struct filesystem* fat16_init();`: function prototype for the `fat16_init` function.
## 11.5. `fat16.c`
This is the meat. Here's where, in the future, things might get tricky. For now, we can breath with ease.
```
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
```
- `void* fat16_fopen(struct disk* disk, struct path_part* path, FILE_MODE mode);`: we make a prototype of `fat16_fopen` here because if we didnt, the next lines won't compile
- `int fat16_resolve(struct disk* disk);`: same here.
- `struct filesystem fat16_fs =`: and this is the line that _would_ cause the issue. nevertheless, it doesn't. we start to create our `fat16_fs` structure.
- `        .resolve = fat16_resolve,`: here we pointer the `resolve` function pointer to `fat16_resolve`.
- `        .open = fat16_fopen`: and here we point `open` to the `fat16_open` functino.
- `struct filesystem* fat16_init()`: function definition.
- `        strcpy(fat16_fs.name, "FAT16");`: here we `strcpy` "FAT16" to the `name` in the struct.
- `        return &fat16_fs;`: and here we return our filesystem to the caller.
- `int fat16_resolve(struct disk* disk)`: function definition. we'll take a `disk` struct.
- `        return 0;`: we return zero for now. this will make the `file.c` `fs_resolve` function assign our FAT16 filesystem to the disk `0`. in the future, we would add a way to check if the disk is, indeed, fat16. baby steps!
- `void* fat16_fopen(struct disk* disk, struct path_part* path, FILE_MODE mode)`: function definition. we'd take a `disk`, a `path` and a file mode.
- `        return 0;`: we return zero for now. we'll implement this later.
## 11.6. `kernel.c`
```
void kernel_main()
{
	....
        // fs init
        fs_init();
        // disk init
        disk_search_and_init();
        ...
}
```
- `        fs_init();`: here we just call the `fs_init` function.
- `        disk_search_and_init();`: this function MUST come after!! if we run this before initializing the filesystem structures, the disk won't have an assigned filesystem and the system will not work properly.
## 11.7. `Makefile`
```
FILES = ... ./build/fs/fat/fat16.o

./build/fs/fat/fat16.o: ./src/fs/fat/fat16.c
        i686-elf-gcc $(INCLUDES) -I./src/fs/ -I./src/fs/fat/ $(FLAGS) -std=gnu99 -c ./src/fs/fat/fat16.c -o ./build/fs/fat/fat16.o
```
Although we have done this before, here we add a new include. Well, not new, but we include the `src/fs` folder instead of just `src/fs/fat`.

And that's it for now!
# 12. Implementing FAT16 Structures
### 12.1.1. Constants
```
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
```
- `#define PEACHOS_FAT16_SIGNATURE 0x29`: this is the FAT signature of our filesystem.
- `#define PEACHOS_FAT16_FAT_ENTRY_SIZE 0x02`: this is the size of each FAT entry in bytes.
- `#define PEACHOS_FAT16_BAD_SECTOR 0xFF7`: this is a cluster value that represents a bad sector.
- `#define PEACHOS_FAT16_UNUSED 0x00`: this is a cluster value that represents an unused sector.
- `typedef unsigned int FAT_ITEM_TIPE;`: here we define an item type. we'll use it later on to define if an item is a directory or an item.
- `#define FAT_ITEM_TYPE_DIRECTORY 0`: we'll use this to define a directory.
- `#define FAT_ITEM_TYPE_FILE 1`: and we'll use this to define a file.
- `#define FAT_FILE_READ_ONLY 0x01`: this attribute represents a read only file.
- `#define FAT_FILE_HIDDEN 0x02`: this attribute represents a hidden file.
- `#define FAT_FILE_SYSTEM 0x04`: this attribute represents a system file and it must not be moved or deleted.
- `#define FAT_FILE_VOLUME_LABEL 0x08`: this attribute represents a volume label.
- `#define FAT_FILE_SUBDIRECTORY 0x10`: this attribute represents a subdirectory.
- `#define FAT_FILE_ARCHIVED 0x20`: this attribute represents a backed up (archived) file.
- `#define FAT_FILE_DEVICE 0x40`: this attribute represents a device file and it must not be changed.
- `#define FAT_FILE_RESERVED 0x80`: this attribute represents the reserved clusters/sectors in disk.
### 12.1.2. `struct fat_header_extended`
```
struct fat_header_extended
{
        uint8_t drive_number;
        uint8_t win_nt_bit;
        uint8_t signature;
        uint32_t volume_id;
        uint8_t volume_id_string[11];
        uint8_t systemd_id_string[8];
} __attribute__((packed));
```
- `struct fat_header_extended`: this structure is a C implementation of the extended BPB. check `boot.asm`.
- `        uint8_t drive_number;`: this is the drive number.
- `        uint8_t win_nt_bit;`: this is the WIN NT bit.
- `        uint8_t signature;`: this is the signature of the FS.
- `        uint32_t volume_id;`: this is the volume id.
- `        uint8_t volume_id_string[11];`: this is the volume ID string.
- `        uint8_t systemd_id_string[8];`: and this is the system ID, `FAT16   `.
- `} __attribute__((packed));`: we pack this structure because it'll be used in the disk.
### 12.1.3. `struct fat_header`
```
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
```
- `struct fat_header`: as before, this is a C implementation of the BPB. check `boot.asm`.
- `        uint8_t short_jmp_ins[3];`: this is the `jump short main` instruction.
- `        uint8_t oem_identifier[8];`: this is the OEMIdentifier.
- `        uint16_t bytes_per_sector;`: this are the bytes per logical sector.
- `        uint8_t sectors_per_cluster;`: this is the sectors per cluster (unused).
- `        uint16_t reserved_sectors;`: this is the amount of reserved sectors we have.
- `        uint8_t fat_copies;`: this is the amount of FAT copies that exist in the FAT table.
- `        uint16_t root_dir_entries;`: this is the amount of root directory entries.
- `        uint16_t number_of_sectors;`: this is number of logical sectors we have.
- `        uint8_t media_type;`: this is the media type.
- `        uint16_t sectors_per_fat;`: this is the number of sectors per FAT table.
- `        uint16_t sectors_per_track;`: this is the number of sectors per track.
- `        uint16_t number_of_heads;`: this is the number of drive heads.
- `        uint32_t hidden_sectors;`: this is the amount of hidden or reserved sectors.
- `        uint32_t sectors_big;`: this is the amount of sectors we actually have.
- `} __attribute__((packed));`: and we pack it up.
### 12.1.4. `struct fat_h`
```
struct fat_h
{
        struct fat_header primary_header;
        union fat_h_e
        {
                struct fat_header_extended extended_header;
        } shared;
};
```
- `struct fat_h`: this is the internal representation that we'll use of the FAT headers.
- `        struct fat_header primary_header;`: this is the BPB.
- `        union fat_h_e`: here we make a union. a union is a special data type that will allow us to store different data types in the same memory location. in practice, it's similar to a structure. this union will only be used if an EBPB is found.
- `                struct fat_header_extended extended_header;`: here we set the extended BPB as a member of this union.
- `        } shared;`: and `shared` is the name of the union. it's the member name that we'll use to access it.
### 12.1.5. `struct fat_directory_item`
```
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
```
- `struct fat_directory_item`: this structure will represent each entry in the FAT table, be it a directory or a file.
- `        uint8_t filename[8];`: filename.
- `        uint8_t ext[3];`: extension of the file.
- `        uint8_t attribute;`: attributes (look above)
- `        uint8_t reserved;`: this is a reserved bit, unused.
- `        uint8_t creation_time_tenths_of_a_sec;`: this is the creation time in miliseconds.
- `        uint16_t creation_time;`: this is the time itself.
- `        uint16_t creation_date;`: this is the creation date.
- `        uint16_t last_access;`: this is the last access date.
- `        uint16_t high_16_bits_first_cluster;`: this are the high 16 bits of the cluster. if it's a directory, it'll represent the next entry in the directory structure. if it's a file, it'll have cluster data, position, etc.
- `        uint16_t last_mod_time;`: this is the last modification time.
- `        uint16_t last_mod_date;`: this is the last modification date.
- `        uint16_t low_16_bits_first_cluster;`: this are the lower 16 bits.
- `        uint32_t filesize;`: and this is the filesize.
- `} __attribute__((packed));`: we pack it up!
### 12.1.6. `struct fat_directory`
```
struct fat_directory
{
        struct fat_directory_item* item;
        int total;
        int sector_pos;
        int ending_sector_pos;
};
```
- `struct fat_directory`: this is a directory entry.
- `        struct fat_directory_item* item;`: here we'll point to the items in the directory. they might be a file or directories.
- `        int total;`: this is the total number of items in the directory.
- `        int sector_pos;`: this is the sector position, or where in the disk the directory is at.
- `        int ending_sector_pos;`: and the last sector (if more than one).
### 12.1.7. `struct fat_item`
```
struct fat_item
{
        union
        {
                struct fat_directory_item* item;
                struct fat_directory* directory;
        };
        FAT_ITEM_TIPE type;
};
```
- `struct fat_item`: this is the file entry.
- `        union`: we create a union.
- `                struct fat_directory_item* item;`: here, we might point to a file or...
- `                struct fat_directory* directory;`: a directory.
- `        FAT_ITEM_TIPE type;`: and the type, be it file or folder.
### 12.1.8. `struct fat_item_descriptor`
```
struct fat_item_descriptor
{
        struct fat_item* item;
        uint32_t pos;
};
```
- `struct fat_item_descriptor`: this is the file descriptor of the item.
- `        struct fat_item* item;`: here we point to the item.
- `        uint32_t pos;`: and here the position in which it exists.
### 12.1.9. `struct fat_private`
```
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
```
- `struct fat_private`: here we define the private data of the FAT filesystem. this data is not supposed to be displayed.
- `        struct fat_h header;`: here we set the FAT header.
- `        struct fat_directory root_directory;`: here the root directory.
- `        struct disk_stream* cluster_read_stream;`: we create a filestream to read clusters.
- `        struct disk_stream* fat_read_stream;`: we create a filestream to read the FAT table.
- `        struct disk_stream* directory_stream;`: we create a filestream to read the directory streams.
For now, it isn't THAT complex. It looks like a lot of code, but it is what is it. FAT16 is one of the simplest filesystems there is, so prepare yourself :)

# 13. Implementing the FAT16 resolver function
This chapter is one of the most complex so far. From here, we'll put to the test everything we've made so far. Our path parser, our I/O functions, our boot FAT sector, our previously made FAT structures, disk streamers and more. As a matter of fact, a couple of thing I made previously were wrong. I mistyped some bytes and did a couple fuck ups. Those we'll fix here, too. But we'll do those at the very end. First, let's go over the FAT resolver implementation.
### 13.1.1. `disk.h`
Here we did a minor change to the `disk` structure. We've added a disk `id`.
```
struct disk
{
        PEACHOS_DISK_TYPE type;
        int sector_size;
        struct filesystem* filesystem;
        int id;
        void* fs_private;
};
```
- `int id`: this `id` will hold the disk ID associated with the disk.
### 13.1.2. `disk.c`
Since we've modified the disk structure, we also need to reflect the changes in the disk initialization routine.
```
void disk_search_and_init()
{
        memset(&disk, 0, sizeof(disk));
        disk.type = PEACHOS_DISK_TYPE_REAL;
        disk.sector_size = PEACHOS_SECTOR_SIZE;
        disk.filesystem = fs_resolve(&disk);
        disk.id = 0;
}
```
  - `      disk.id = 0;`: here we'll set 0 for now. Remember that as of right now, we don't have a way to detect disks and other stuff. Just disk 0.
## fat16.c
Here's where we get into the resolver function. We'll go over it step by step, from order of execution in the main `fat16_resolve` function.
### 13.2.1. `static void fat16_init_private(struct disk* disk, struct fat_private* private)`
Similar to all our other `init` functions, this function will initialize the private structures of our FAT filesystems. This data contains the FAT header, FAT extended header, data streams and other stuff. We initialize the streams, as of right now. We'll get the sector information later on.
```
static void fat16_init_private(struct disk* disk, struct fat_private* private)
{
        memset(private, 0, sizeof(struct fat_private));
        private->cluster_read_stream = diskstreamer_new(disk->id);
        private->fat_read_stream = diskstreamer_new(disk->id);
        private->directory_stream = diskstreamer_new(disk->id);
}
```
- `static void fat16_init_private(struct disk* disk, struct fat_private* private)`: function definition. we'll take a disk and a previously defined `fat_private` structure.
- `        memset(private, 0, sizeof(struct fat_private));`: here we memset the entire data region to zero.
- `        private->cluster_read_stream = diskstreamer_new(disk->id);`: here we initialize the cluster stream.
- `        private->fat_read_stream = diskstreamer_new(disk->id);`: here we initialize the FAT reading stream.
- `        private->directory_stream = diskstreamer_new(disk->id);`: here we initialize the directory stream.
### 13.2.2. `int fat16_get_root_directory(struct disk* disk, struct fat_private* fat_private, struct fat_directory* directory)`
Now here we're starting to get serious. The `fat16_get_root_directory` function will do several calls to get the root directory and other fun stuff.
```
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
```
- `int fat16_get_root_directory(struct disk* disk, struct fat_private* private, struct fat_directory* directory)`: function definition. we'll take a disk, a fat_private and a directory structure. the directory structure is inside the `fat_private` struct.
- `        int res = 0;`: here we set the initial return value.
- `        struct fat_header* primary_header = &private->header.primary_header;`: here we create a pointer to the `primary_header` (FAT header). it'll be easier to work with it this way.
- `        int root_dir_sector_pos = (primary_header->fat_copies * primary_header->sectors_per_fat) + primary_header->reserved_sectors;`: here we calculate the root directory by multiplying the amount of FAT table copies with the sectors per FAT value (256 or `0x100`). this is then added to the amount of `reserved_sectors`. this formula will give us the exact place in which the `root_directory` should be positioned.
- `        int root_directory_entries = private->header.primary_header.root_dir_entries;`: here we get the amount of root directory entries from the primary header.
- `        int root_directory_size = (root_directory_entries * sizeof(struct fat_directory_item));`: here we get the total directory size by multiplying the `root_directory_entries` with the size of the `fat_directory_item` struct.
- `        int total_sectors = root_directory_size / disk->sector_size;`: here we get the total amount of sectors used by the `root_directory_size` (bytes value) and dividing it by the logical `sector_size` defined in our `disk`.
- `        if(root_directory_size % disk->sector_size)`: here we check if the `root_directory_size` needs alignment. if so...
- `                total_sectors += 1;`: we add `1` to the total sector count.
- `        int total_items = fat16_get_total_items_per_directory(disk, root_dir_sector_pos);`: here we call the `fat16_get_total_items` function. we'll see that function right after this one. in short: it returns the amount of _stuff_ in the root directory. be it files or directories.
- `        struct fat_directory_item* dir = kzalloc(sizeof(root_directory_size));`: here we create and allocate zero'd out memory for the `root_directory_size` size.
- `        if(!dir)`: here we check if the allocation failed. if so...
- `                res = -ENOMEM;`: return `-ENOMEM`.
- `                goto out;`: and go to the `out` label.
- `        struct disk_stream* stream = private->directory_stream;`: here we access the previously initialized `directory_stream`
- `        if(diskstreamer_seek(stream, fat16_sector_to_absolute(disk, root_dir_sector_pos)) != PEACHOS_ALLOK)`: here we `seek` to the `fat16_sector_to_absolute` byte, which we'll see later on. for now, just keep in mind that we're multiplying the sector value contained in `root_dir_sector_pos` to the byte value usable by the `diskstreamer_read`. if this call fails...
- `                res = -EIO;`: we return `-EIO`
- `                goto out;`: and we go to the `out` label.
- `        if(diskstreamer_read(stream, dir, root_directory_size) != PEACHOS_ALLOK)`: here we read the bytes set in the `diskstreamer_seek` function, which size is `root_directory_size` into the `dir` buffer. if this call fails...
- `                res = -EIO;`: we return `-EIO`
- `                goto out;`: and we go to the `out` label.
- `        directory->item = dir;`: here we set the `directory` item to `dir`.
- `        directory->total = total_items;`: here we set the `total` to `total_items`.
- `        directory->sector_pos = root_dir_sector_pos;`: here we set the `sector_pos` to `root_dir_sector_pos`.
- `        directory->ending_sector_pos = root_dir_sector_pos + (root_directory_size / disk->sector_size);`: and the `ending_sector_pos` is a calculation in which we sum the ending sector position with the `root_directory_size` divided by the sector size in our `disk`.
- `out:`: `out` label.
- `        return res;`: and here we return res, which, if everything went well, will be zero.
### 13.2.3. `int fat16_get_total_items_for_directory(struct disk* disk, uint32_t directory_start_sector)`
Here we'll see how we can calculate the amount of _stuff_ held in a directory.
```
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
```
- `int fat16_get_total_items_per_directory(struct disk* disk, uint32_t directory_start_sector)`: here we'll get the `disk` and the starting sector from which we'll get the directory total.
- `        struct fat_directory_item item;`: here we define a `fat_directory_item`.
- `        struct fat_directory_item empty_item;`: here we define another one, but it won't be used.
- `        memset(&empty_item, 0, sizeof(empty_item));`: here we memset the empty directory to zero.
- `        struct fat_private* private = disk->fs_private;`: here we access the private data contained by our filesystem.
- `        int res = 0;`: we set the `res` return value.
- `        int i = 0;`: and a counter, which we'll use to count how many _things_ are in a directory.
- `        int directory_start_pos = directory_start_sector * disk->sector_size;`: here we get the initial position in bytes by multiplying the `directory_start_sector` with the disk's logical `sector_size`.
- `        struct disk_stream* stream = private->directory_stream;`: here we access the previously initialized `directory_stream` `stream`.
- `        if(diskstreamer_seek(stream, directory_start_pos) != PEACHOS_ALLOK)`: here we `seek` to the `directory_start_pos`. if something goes wrong...
- `                res = -EIO;`: we return `-EIO`.
- `                goto out;`: and we go to `out`.
- `        while(1)`: now, we'll need an infinite loop to read over the items we have in the directories. 
- `                if(diskstreamer_read(stream, &item, sizeof(item)) != PEACHOS_ALLOK)`: here we read from the previous `seek` byte into `&item` with a size of `item`. `&item`, after each iteration, will grow in size. if the read function goes wrong...
- `                        res = -EIO;`: we return `-EIO`.
- `                        goto out;`: and we go to `out`.
- `                if (item.filename[0] == 0x00)`: here we check if the `filename` starts with null bytes. if so, we have read over everything.
- `                        break;`: and we break the loop.
- `                if (item.filename[0] == 0xE5)`: if `0xE5`, we've found something. 
- `                        continue;`: we continue.
- `                i++;`: and we add an item to the `i` counter.
- `        res = i;`: here we set `res` to `i`
- `out:`: `out` label.
- `        return res;`: and we return `res`.
### 13.2.4. `int fat16_sector_to_absolute(struct disk* disk, int sector)`
This function is quite simple, as previously stated.
```
int fat16_sector_to_absolute(struct disk* disk, int sector)
{
        return sector * disk->sector_size;
}
```
- `int fat16_sector_to_absolute(struct disk* disk, int sector)`: function definition. we'll take a disk and a sector.
- `        return sector * disk->sector_size;`: we return the sector multiplied by the `sector_size`.
### 13.2.5. `int fat16_resolve(struct disk* disk)`
Here's where everything before comes together. Not really. At least not for now.
```
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
```
- `int fat16_resolve(struct disk* disk)`: function definition. we'll just take a disk.
- `        int res = 0;`: we set the initial return value.
- `        struct fat_private* fat_private = kzalloc(sizeof(struct fat_private));`: here we allocate some memory on the heap for the `fat_private` data structure.
- `        fat16_init_private(disk, fat_private);`: here we initialize the `fat_private`, as seen previously.
- `        disk->fs_private = fat_private;`: here we set the `disk` private data to the `fat_private`.
- `        disk->filesystem = &fat16_fs;`: and here we assign the filesystem to us, for now.
- `        struct disk_stream* stream = diskstreamer_new(disk->id);`: here we create a new stream.
- `        if(!stream)`: if the stream wasn't created successfully...
- `                res = -ENOMEM;`: we return `-ENOMEM`.
- `                goto out;`: and go to `out`.
- `        if(diskstreamer_read(stream, &fat_private->header, sizeof(fat_private->header)) != PEACHOS_ALLOK)`: by not using `seek`, we're reading at byte 0 into the `&fat_private->header` with a size of `fat_private->header`. if something happened...
- `                res = -EIO;`: we return `-EIO`.
- `                goto out;`: and we go to `out`.
- `        if (fat_private->header.shared.extended_header.signature != 0x29)`: to check that the filesystem is indeed us, we'll read the `fat_private->header.shared.extended_header.signature` value, which should be `0x29`. if not...
- `                res = -EFSNOTUS;`: we return `-EFSNOTUS`.
- `                goto out;`: and go to `out`.
- `        if (fat16_get_root_directory(disk, fat_private, &fat_private->root_directory) != PEACHOS_ALLOK)`: and here we get the root directory. and set it to `&fat_private->root_directory`. if something goes wrong...
- `                res = -EIO;`: we return `-EIO`.
- `                goto out;`: and we go to `out`.
- `out:`: `out` label.
- `        if(stream)`: here we check if `stream` is opened. if so...
- `                diskstreamer_close(stream);`: we close it.
- `        if(res < 0)`: and here we check fi `res` is less than zero. if so, something went wrong, and we should...
- `                kfree(fat_private);`: free the memory used by `fat_private` 
- `                disk->fs_private = 0;`: and reset the `fs_private` data.
- `        return res;`: and we return `res`. which, if zero, is OK; if not, NOT OK.
And that's it! We have working resolver function. Not really. If you were following step by step, you'll have issues. So, let's fix them.
## 13.3. Fixing the fuckups.
### 13.3.1. `io.asm`
So when making the `io.asm` labels, I mistyped some registers.
```
insw:
	...
-       in al, dx
+       in ax, dx
	...
```
Yes. We were reading byte (singular) instead of bytes (plural) in an `insw`, where we should be reading TWO bytes. I debugged this for like 4 hours.
### 13.3.1. `boot.asm`
Here I also mistyped some bytes.
```
-SectorsBig             dd 0x773544
+SectorsBig             dd 0x773594
```
Yes. Instead of a `9`, I wrote a `4`. As before, it took me HOURS to find this. But hey, you live you learn.
### 13.3.1. `disk.c`
And finally...
```
-       outb(0x1F4, (unsigned char) lba >> 8);
-       outb(0x1F4, (unsigned char) lba >> 16);
+       outb(0x1F4, (unsigned char)(lba >> 8));
+       outb(0x1F5, (unsigned char)(lba >> 16));
```
The major bug was only using the `0x1F4` interface to send the LBA values. I NEEDED to use the `0x1F5` too, alongside `0x1F3`. Nevertheless, it's working now.

# 14. Implementing the VFS `fopen`.
Now, we need to implement the basis of our `fopen` function at the VFS layer. This doesn't mean that we have a working FAT read function, no. We're making the structure for our virtual filesystem layer to be able to handle the calls to FAT16 and any other filesystem that might use it.
## 14.1. `string.c`
Before doing the `fopen` basic structure, we need a couple tools for that.
### 14.1.1. `int strncmp(const char* str1, const char* str2, int n)`
We'll implement a simple `string compare` function.
```
int strncmp(const char* str1, const char* str2, int n)
{
        unsigned char u1, u2;
        while(n-- > 0)
        {
                u1 = (unsigned char)*str1++;
                u2 = (unsigned char)*str2++;
                if (u1 != u2)
                {
                        return u1 - u2;
                }
                if (u1 == '\0')
                {
                        return 0;
                }
        }
        return 0;
}
```
- `int strncmp(const char* str1, const char* str2, int n)`: here we'll get two strings to compare and an `n` number of bytes to compare.
- `        unsigned char u1, u2;`: here we'll use two unsigned chars to work with the `str{1,2}` as pointers.
- `        while(n-- > 0)`: here `while` loop until `n--` (reading all but one) value isn't zero.
- `                u1 = (unsigned char)*str1++;`: we set `u1` to the first character of `*str1` to it and we increment `str1`.
- `                u2 = (unsigned char)*str2++;`: we set `u2` to the first character of `*str2` to it and we increment `str2`.
- `                if (u1 != u2)`: if `u1` and `u2` aren't equal...
- `                        return u1 - u2;`: we return `u1 - u2`, which isn't zero.
- `                if (u1 == '\0')`: we check if we arrived at a NULL terminator. if so...
- `                        return 0;`: we return zero.
- `        return 0;`: and we return zero here in case the `while` loop finishes before the NULL terminator.
### 14.1.2. `int strnlen_terminator(const char* str, int max, char terminator)`
This function will be used to check the lenght of strings with a `max` amount of characters to read and checking for a `terminator` and breaking at it.
```
int strnlen_terminator(const char* str, int max, char terminator)
{
        int i = 0;
        for(i = 0; i < max++; i++)
        {
                if (str[i] == '\0' || str[i] == terminator)
                {
                        break;
                }
        }
        return i;
}
```
- `int strnlen_terminator(const char* str, int max, char terminator)`: function definition. we'll get a string, a `max` bytes to read and a `terminator` value.
- `        int i = 0;`: here we set the initial counter to zero.
- `        for(i = 0; i < max; i++)`: here we'll run a `for` loop until `max`.
- `                if (str[i] == '\0' || str[i] == terminator)`: we check if if value at `str[i]` is a NULL terminator or the `terminator` value. if so...
- `                        break;`: we break.
- `        return i;`: and here we return the number of characters read until the `terminator` or the NULL byte.
### 14.1.3. `char tolower(char s1)`
Since the cringe FAT filesystem does not distinguish between upper and lower case, we need a way to make everything lowercase.
```
char tolower(char s1)
{
        if(s1 >= 65 && s1 <= 90)
        {
                s1 += 32;
        }
        return s1;
}
```
- `char tolower(char s1)`: function definition. we'll take a single char.
- `        if(s1 >= 65 && s1 <= 90)`: here we check if the `char` is between `65` and `90`. check the ASCII table to see why :) if the char is between these values...
- `                s1 += 32;`: we increment it by 32, effectively making it lowercase.
- `        return s1;`: and we return `s1`.
### 14.1.4. `int istrncmp(const char* s1, const char* s2, int n)`
We'll also need an case insensitive `strncmp`.
```
int istrncmp(const char* s1, const char* s2, int n)
{
        unsigned char u1, u2;
        while(n-- > 0)
        {
                u1 = (unsigned char)*s1++;
                u2 = (unsigned char)*s2++;
                if(u1 != u2 && tolower(u1) != tolower(u2))
                {
                        return u1 - u2;
                }
                if (u1 == '\0')
                {
                        return 0;
                }
        }
        return 0;
}
```
- `int istrncmp(const char* s1, const char* s2, int n)`: as before, we'll take two strings and an `n` amount of bytes to compare.
- `        unsigned char u1, u2;`: we make two unsigned chars to use `s1` and `s2` as pointers.
- `        while(n-- > 0)`: `while` loop that continually decrements `n` as it goes until it's lower than zero.
- `                u1 = (unsigned char)*s1++;`: here we set `u1` to the first character of the `s1` casted pointer and we increment it.
- `                u2 = (unsigned char)*s2++;`: here we set `u2` to the first character of the `s2` casted pointer and we increment it.
- `                if(u1 != u2 && tolower(u1) != tolower(u2))`: we check if `u1` and `u2` are not the same and if converting them to lowercase is also not the same. fi so...
- `                        return u1 - u2;`: we return `u1` minus `u2`.
- `                if (u1 == '\0')`: and if we hit a null terminator...
- `                        return 0;`: we return zero.
- `        return 0;`: and we also return zero outside, in case we don't hit a null terminator.
## 14.2. `string.h`
Here we'll just put our prototypes.
```
int strncmp(const char* str1, const char* str2, int n);
int strnlen_terminator(const char* str, int max, char terminator);
int istrncmp(const char* s1, const char* s2, int n);
char tolower(char s1);
```
## 14.3. `file.c`
Before actually getting into the `fopen` function, we need to check something else hehe.
### 14.3.1. `FILE_MODE file_get_mode_by_string(const char* str)`
We'll make a function that converts our `mode_string` (more on that later) into an anctual `FILE_MODE` type.
```
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
```
- `FILE_MODE file_get_mode_by_string(const char* str)`: here we'll receive the `mode_string`.
- `        FILE_MODE mode = FILE_MODE_INVALID;`: we'll set the initial return value to `FILE_MODE_INVALID`.
- `        if(strncmp(str, "r", 1) == 0)`: here we check if the initial byte of the `mode_string` is `r`. if so...
- `                mode = FILE_MODE_READ;`: we set `FILE_MODE_READ.`
- `        if(strncmp(str, "w", 1) == 0)`: here we check if the initial byte of the `mode_string` is `w`. if so...
- `                mode = FILE_MODE_WRITE;`: we set `FILE_MODE_WRITE.`
- `        if(strncmp(str, "a", 1) == 0)`: here we check if the initial byte of the `mode_string` is `a`. if so...
- `                mode = FILE_MODE_APPEND;`: we set `FILE_MODE_APPEND.`
- `        return mode;`: and we return the mode.
### 14.3.2. `int fopen(const char* filename, const char* mode_string)`
And finally! The `fopen` basis! Let's see it!
```
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
```
- `int fopen(const char* filename, const char* mode_string)`: we'll take a filename (path) and a mode.
- `        int res = 0;`: here we set the initial return value.
- `        struct path_root* root_path = pathparser_parse(filename, NULL);`: here we'll make a `path_root` using the `pathparser_parse` using the provided `filename` and without a current working directory (NULL).
- `        if(!root_path)`: if something went wrong with the parsing...
	- something that could've went wrong is that the user passed "0:/" and no file.
- `                res = -EINVARG;`: we return `-EINVARG`.
- `                goto out;`: and we go to `out`.
- `        if(!root_path->first)`: here we check if the `root_path` has a next element, that is, that we're not pointing at the root directory itself. if so...
- `                res = -EINVARG;`: we return `-EINVARG`.
- `                goto out;`: and we go to `out`.
- `        struct disk* disk = disk_get(root_path->drive_no);`: here we get the disk structure found in the `root_path`.
- `        if(!disk)`: we check if the disk not zero. if so...
- `                res = -EIO;`: we return `-EIO`.
- `                goto out;`: and we go to `out`.
- `        if(!disk->filesystem)`: here we check if the `disk` structure has a `filesystem` associated with it. if it doesn't...
- `                res = -EIO;`: we return `-EIO`.
- `                goto out;`: and we go to `out`.
- `        FILE_MODE mode = file_get_mode_by_string(mode_string);`: here we get the file open mode with the previously described function.
- `        if(mode == FILE_MODE_INVALID)`: here we check if the `mode` is invalid. if so...
- `                res = -EINVARG;`: we return `-EINVARG`.
- `                goto out;`: and we go to `out`.
- `        void* descriptor_private_data = disk->filesystem->open(disk, root_path->first, mode);`: here we initialize a void pointer that will hold the value of the `open` function pointer.
- `        if(ISERR(descriptor_private_data))`: here we check `ISERR`. we'll see what this is later on. know for now that it's a macro that checks if the value is less than zero. if it is...
- `                res = ERROR_I(descriptor_private_data);`: we res `res` to `ERROR_I`, which is just a macro that casts the error value to an integer.
- `                goto out;`: and we go to `out`.
- `        struct file_descriptor* desc = 0;`: here we create a new file descriptor.
- `        res = file_new_descriptor(&desc);`: here we initialize the file descriptor structures.
- `        if(res < 0)`: here we check if the `res` value is less than zero. if so...
- `                goto out;`: we go to `out`.
- `        desc->filesystem = disk->filesystem;`: here we set the new file descriptor's filesystem to the one we have in the `disk` structure.
- `        desc->private = disk->fs_private;`: here we set the new file descriptor's private data to the one we have in the `disk` structure.
- `        desc->disk = disk;`: and we set the file descriptor's `disk` to the `disk` strucutre.
- `        res = desc->index;`: and we set `res` to the newly created file descriptor index.
- `out:`: `out` label.
- `        if(res < 0)`: we check if `res` is lower than zero. if so, no file descriptor was created and...
- `                res = 0;`: we set it to zero.
- `        return res;`: and we return.
In this last two chapters, we've really made use of everything we've made thus far. Path parsers, disk streamers, disk initailizations, heap implementation, and more. Awesome, right?
## 14.4.1. `kernel.h`
We've defined some new macros in the `kernel.h` header.
```
#define ERROR(valud) (void*)(value)
#define ERROR_I(value) (int)(value)
#define ISERR(value) ((int)value < 0)
```
- `#define ERROR(valud) (void*)(value)`: here we take a value and we cast it to a void pointer.
- `#define ERROR_I(value) (int)(value)`: here we take a value and we cast it to an integer.
- `#define ISERR(value) ((int)value < 0)`: and here we take a value and return true or false depending on if `value` is less than zero or not.

