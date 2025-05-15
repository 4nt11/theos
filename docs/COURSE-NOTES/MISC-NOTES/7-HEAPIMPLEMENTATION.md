- Memory map https://wiki.osdev.org/Memory_Map_(x86)
## 1.8 The entry table
- Its composed of an array of 1 byte values that represent an entry in our heap data pool.
- Array size is calculated by taking the heap data pool size and dividing it by our block size of 4096 bytes. We are left with the total number fo entries we need in our array.
- For example:
	- We want a 100MB heap then the math would be:
		- 100MB / 4096 = 25600 bytes in our entry table.
	- If our heap data pool is at address `0x01000000` then entry zero in our table will represent address `0x01000000`.
	- Entry one will represent address `0x01001000`.
	- Entry two will represent address `0x01002000`.
	- `0x1000`: 4096 bytes.
## 1.9 The entry structure
Upper 4 bits are flags.
			Lower 4 bits represent the entry type.

| HAS_N | IS_FIRST | 0   | 0   | ET_3 | ET_2 | ET_1 | ET_0 |
| ----- | -------- | --- | --- | ---- | ---- | ---- | ---- |
- `HAS_N`: Set if the entry to the right of us is part of our allocation.
	- If we allocate 2 blocks of memory, than entry 0 would have the `HAS_N` bit set, and block 1 would not.
- `IS_FIRST`: Set if this is the first entry of our allocation.
- 00: Unused.
- **Each entry byte describes 4096 bytes of data in the heap data pool**.

- `0xC1 | 1100 0001`: Block taken, first block, has more blocks for this allocation.
- `0x41 | 0100 0001`: Block taken, first block, no more blocks for this allocation.
- `0x81 | 1000 0001`: Block taken and we have more blocks for this allocation, we are not the first block for this allocation.
- `0x01 | 0000 0001`: Block taken, we are not the first block for this allocation, no more blocks for this allocation.
- `0x00 | 0000 0000`: Block free or (lower 4 bits = 0) block free.

## 1.10 Entry types
- `HEAP_BLOCK_ENTRY_TAKEN`: The entry is taken and the address cannot be used.
- `HEAP_BLOCK_ENTRY_FREE`: The entry is free and may be used.


- `0xC1 | 1100 0001`: Block taken, first block, has more blocks for this allocation.
- `0x41 | 0100 0001`: Block taken, first block, no more blocks for this allocation.
- `0x81 | 1000 0001`: Block taken and we have more blocks for this allocation, we are not the first block for this allocation.
- `0x01 | 0000 0001`: Block taken, we are not the first block for this allocation, no more flobkc for this allocation.
- `0x00 | 0000 0000`: Block free or (lower 4 bits = 0) block free.

## Absolute address calculation from array index.
`(array_position * BLOCK_SIZE) = offset`
If our heap data pool starts at address `0x01000000`, we add the offset.
`initial_address + offset = absolute_address`.

# Sections of memory
| start                                   | end                                     | size                                    | description                             | type                                    | type                                       |
| --------------------------------------- | --------------------------------------- | --------------------------------------- | --------------------------------------- | --------------------------------------- | ------------------------------------------ |
| Real mode address space (the first MiB) | Real mode address space (the first MiB) | Real mode address space (the first MiB) | Real mode address space (the first MiB) | Real mode address space (the first MiB) | Real mode address space (the first MiB)    |
| 0x00000000                              | 0x000003FF                              | 1 KiB                                   | Real Mode IVT (Interrupt Vector Table)  | unusable in real mode                   | 640 KiB RAM ("Low memory")                 |
| 0x00000400                              | 0x000004FF                              | 256 bytes                               | BDA (BIOS data area)                    | unusable in real mode                   | 640 KiB RAM ("Low memory")                 |
| 0x00000500                              | 0x00007BFF                              | 29.75 KiB                               | Conventional memory                     | usable memory                           | 640 KiB RAM ("Low memory")                 |
| 0x00007C00                              | 0x00007DFF                              | 512 bytes                               | Your OS BootSector                      | usable memory                           | 640 KiB RAM ("Low memory")                 |
| 0x00007E00                              | 0x0007FFFF                              | 480.5 KiB                               | Conventional memory                     | usable memory                           | 640 KiB RAM ("Low memory")                 |
| 0x00080000                              | 0x0009FFFF                              | 128 KiB                                 | EBDA (Extended BIOS Data Area)          | partially used by the EBDA              | 640 KiB RAM ("Low memory")                 |
| 0x000A0000                              | 0x000BFFFF                              | 128 KiB                                 | Video display memory                    | hardware mapped                         | 384 KiB System / Reserved ("Upper Memory") |
| 0x000C0000                              | 0x000C7FFF                              | 32 KiB (typically)                      | Video BIOS                              | ROM and hardware mapped / Shadow RAM    | 384 KiB System / Reserved ("Upper Memory") |
| 0x000C8000                              | 0x000EFFFF                              | 160 KiB (typically)                     | BIOS Expansions                         | ROM and hardware mapped / Shadow RAM    | 384 KiB System / Reserved ("Upper Memory") |
| 0x000F0000                              | 0x000FFFFF                              | 64 KiB                                  | Motherboard BIOS                        | ROM and hardware mapped / Shadow RAM    | 384 KiB System / Reserved ("Upper Memory") |
# Extended Memory Map
|start|end|size|region/exception|description|
|---|---|---|---|---|
|High Memory|   |   |   |   |
|0x00100000|0x00EFFFFF|0x00E00000 (14 MiB)|RAM -- free for use (if it exists)|Extended memory 1, 2|
|0x00F00000|0x00FFFFFF|0x00100000 (1 MiB)|Possible memory mapped hardware|ISA Memory Hole 15-16MB 3|
|0x01000000|????????|???????? (whatever exists)|RAM -- free for use|More Extended memory 1|
|0xC0000000 (sometimes, depends on motherboard and devices)|0xFFFFFFFF|0x40000000 (1 GiB)|various (typically reserved for memory mapped devices)|Memory mapped PCI devices, PnP NVRAM?, IO APIC/s, local APIC/s, BIOS, ...|
|0x0000000100000000 (possible memory above 4 GiB)|????????????????|???????????????? (whatever exists)|RAM -- free for use (PAE/64bit)|More Extended memory 1|
|????????????????|????????????????|????????????????|Possible memory mapped hardware|Potentially usable for memory mapped PCI devices in modern hardware (but typically not, due to backward compatibility)|
# Calculating ending address.
Take the total size in bytes (104857600) and add it to the base address (0x01000000).
0x01000000 + 104857600
0x01000000 + 0x06400000 = 121634816, or 0x07400000

# Calculating total heap table size.
Take the end and start address and get the difference.
0x07400000 - 0x01000000 = 104857600, which is the same size in bytes.

# Calculating total block number.
Take the size in bytes and divide it by the block size.
104857600 / 4096 = 25600

# Aligning heap allocation sizes.
In our current system, we can only allocate in blocks of a given size, 4096 in our case. A developer may not use (or know) about this, and so he might try to use values like `50`, `100` or `10000`. If so, we need to work in aligning the values to the block size we're using. For that, take the following formula:
`(allocation - (allocation % BLOCK_SIZE)) + BLOCK_SIZE` would align the values. Let's see an example:
`(50 - ( 50 % 4096)) + 4096`
`(50 - 50) + 4096`
`0 + 4096`
`4096`
And done.
# Calculating block addresses.
This calculation is fairly simple. The general formula is the following:
`starting_address + (block_count * block_size)`
For example:
`0x01000000 + (10 * 4096)`
`0x01000000 + 40960`
`0x0100A000`
# Calculating entry types.
To calculate the entry type, we need to understand bitwise operations. In the case of `heap_get_entry_type`, we `AND` the entry with `0x0f`. but, why is that?
We have to main entry types:
- 0000 000***1*** for taken entries, and
- 0000 000***0*** for free entries.
Since we're working with bits, `0x0f` translates to `0000 1111` and the zero before the F MUST be used.
So, if we have a taken entry...
`00000001 & 00001111` is...
`00000001`
And...
`00000000 & 00001111` is...
`00000000`.
This is because in `AND` operations, both bits on the operation must be set for it to be passed over. Unlike `OR`, where only one bit can be set instead of both.
# Heap address to block number.
We'll also need to calculate the block number from a pointer address at some point. Let's see how.

`(addr - start_addr)) / PEACHOS_HEAP_BLOCK_SIZE`
Say we memory allocated at `0x01001000` (1 block) and our heap starts at `0x01000000`. Let's see how to use the formula.
`(0x01001000 - 0x01000000) - 4096`
`0x1000 - 4096`
`4096 - 4096`
`0`
And indeed, `0x01001000` is the first (0) block of our array.
