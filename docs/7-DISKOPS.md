# Description
This page details some information on the current state of disk operations. This isn't a filesystem specific page, but rather an agnostic page on drive reading, writting and general access.
# Related code
The related code can be found on the following directories:
- src/disk
- src/disk/disk.c
- src/disk/disk.h
# Architecture
We've implemented a small driver that will allow us to read `n` sectors using the LBA ATA ports.
## `int disk_read_sector(int lba, int total, void* buffer)`
This code will take an `lba` start sector, a `total` amount of sectors to be read starting from `lba` and a `buffer[SIZE]` in which the read sectors will be read to. This is done using the previously implemented I/O instructions `outb` and `insw`.
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
## `void disk_search_and_init()`
This function will initialize the `disk` struct and assign some types. It's in early alpha, since it doesn't really search for anything right now.
```
void disk_search_and_init()
{
        memset(&disk, 0, sizeof(disk));
        disk.type = PEACHOS_DISK_TYPE_REAL;
        disk.sector_size = PEACHOS_SECTOR_SIZE;
}
```
## `struct disk* disk_get(int index)`
This function will get you the disk from an index. Again, it in early alpha, since right now all it does is return the already existing `disk` structure as a pointer.
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
## `int disk_read_block(struct disk* idisk, unsigned int lba, int total, void* buf)`
This function will be the main way in which a programmer will read sectors from the disk. It requires a `disk` structure which is obtained by using `disk-get`, an `lba` starting sector, the `total` sectors to be read and a pointer to a `buf` in which the read data will be stored.
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
