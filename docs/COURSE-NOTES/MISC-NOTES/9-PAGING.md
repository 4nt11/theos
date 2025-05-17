# Attributes

![[Pasted image 20250514221210.png]]

### Page Directory

The topmost paging structure is the page directory. It is essentially an array of page directory entries that take the following form.

[![](https://wiki.osdev.org/images/1/1e/Page_directory_entry.png)](https://wiki.osdev.org/File:Page_directory_entry.png)

A Page Directory Entry

When PS=0, the page table address field represents the physical address of the page table that manages the four megabytes at that point. Please note that it is very important that this address be 4-KiB aligned. This is needed, due to the fact that the last 12 bits of the 32-bit value are overwritten by access bits and such. Similarly, when PS=1, the address must be 4-MiB aligned.

- PAT, or '**P**age **A**ttribute **T**able'. If [PAT](https://en.wikipedia.org/wiki/Page_attribute_table) is supported, then PAT along with PCD and PWT shall indicate the memory caching type. Otherwise, it is reserved and must be set to 0.
- G, or '**G**lobal' tells the processor not to invalidate the TLB entry corresponding to the page upon a MOV to CR3 instruction. Bit 7 (PGE) in CR4 must be set to enable global pages.
- PS, or '**P**age **S**ize' stores the page size for that specific entry. If the bit is set, then the PDE maps to a page that is 4 MiB in size. Otherwise, it maps to a 4 KiB page table. Please note that 4-MiB pages require PSE to be enabled.
- D, or '**D**irty' is used to determine whether a page has been written to.
- A, or '**A**ccessed' is used to discover whether a PDE or PTE was read during virtual address translation. If it has, then the bit is set, otherwise, it is not. Note that, this bit will not be cleared by the CPU, so that burden falls on the OS (if it needs this bit at all).
- PCD, is the 'Cache Disable' bit. If the bit is set, the page will not be cached. Otherwise, it will be.
- PWT, controls Write-Through' abilities of the page. If the bit is set, write-through caching is enabled. If not, then write-back is enabled instead.
- U/S, the '**U**ser/Supervisor' bit, controls access to the page based on privilege level. If the bit is set, then the page may be accessed by all; if the bit is not set, however, only the supervisor can access it. For a page directory entry, the user bit controls access to all the pages referenced by the page directory entry. Therefore if you wish to make a page a user page, you must set the user bit in the relevant page directory entry as well as the page table entry.
- R/W, the '**R**ead/**W**rite' permissions flag. If the bit is set, the page is read/write. Otherwise when it is not set, the page is read-only. The WP bit in CR0 determines if this is only applied to userland, always giving the kernel write access (the default) or both userland and the kernel (see Intel Manuals 3A 2-20).
- P, or '**P**resent'. If the bit is set, the page is actually in physical memory at the moment. For example, when a page is swapped out, it is not in physical memory and therefore not 'Present'. If a page is called, but not present, a page fault will occur, and the OS should handle it. (See below.)

The remaining bits 9 through 11 (if PS=0, also bits 6 & 8) are not used by the processor, and are free for the OS to store some of its own accounting information. In addition, when P is not set, the processor ignores the rest of the entry and you can use all remaining 31 bits for extra information, like recording where the page has ended up in swap space. When changing the accessed or dirty bits from 1 to 0 while an entry is marked as present, it's recommended to invalidate the associated page. Otherwise, the processor may not set those bits upon subsequent read/writes due to TLB caching.

[![](https://wiki.osdev.org/images/6/60/Page_table_entry.png)](https://wiki.osdev.org/File:Page_table_entry.png)

A Page Table Entry

Setting the PS bit makes the page directory entry point directly to a 4-MiB page. There is no paging table involved in the address translation. Note: With 4-MiB pages, whether or not bits 20 through 13 are reserved depends on PSE being enabled and how many PSE bits are supported by the processor (PSE, PSE-36, PSE-40). [CPUID](https://wiki.osdev.org/CPUID "CPUID") should be used to determine this. Thus, the physical address must also be 4-MiB-aligned. Physical addresses above 4 GiB can only be mapped using 4 MiB PDEs.

## Page directory attributes
- PAT, or '**P**age **A**ttribute **T**able'. If [PAT](https://en.wikipedia.org/wiki/Page_attribute_table) is supported, then PAT along with PCD and PWT shall indicate the memory caching type. Otherwise, it is reserved and must be set to 0.
- G, or '**G**lobal' tells the processor not to invalidate the TLB entry corresponding to the page upon a MOV to CR3 instruction. Bit 7 (PGE) in CR4 must be set to enable global pages.
- PS, or '**P**age **S**ize' stores the page size for that specific entry. If the bit is set, then the PDE maps to a page that is 4 MiB in size. Otherwise, it maps to a 4 KiB page table. Please note that 4-MiB pages require PSE to be enabled.
- D, or '**D**irty' is used to determine whether a page has been written to.
- A, or '**A**ccessed' is used to discover whether a PDE or PTE was read during virtual address translation. If it has, then the bit is set, otherwise, it is not. Note that, this bit will not be cleared by the CPU, so that burden falls on the OS (if it needs this bit at all).
- PCD, is the 'Cache Disable' bit. If the bit is set, the page will not be cached. Otherwise, it will be.
- PWT, controls Write-Through' abilities of the page. If the bit is set, write-through caching is enabled. If not, then write-back is enabled instead.
- U/S, the '**U**ser/Supervisor' bit, controls access to the page based on privilege level. If the bit is set, then the page may be accessed by all; if the bit is not set, however, only the supervisor can access it. For a page directory entry, the user bit controls access to all the pages referenced by the page directory entry. Therefore if you wish to make a page a user page, you must set the user bit in the relevant page directory entry as well as the page table entry.
- R/W, the '**R**ead/**W**rite' permissions flag. If the bit is set, the page is read/write. Otherwise when it is not set, the page is read-only. The WP bit in CR0 determines if this is only applied to userland, always giving the kernel write access (the default) or both userland and the kernel (see Intel Manuals 3A 2-20).
- P, or '**P**resent'. If the bit is set, the page is actually in physical memory at the moment. For example, when a page is swapped out, it is not in physical memory and therefore not 'Present'. If a page is called, but not present, a page fault will occur, and the OS should handle it. (See below.)

Source: [OsDev](https://wiki.osdev.org/Paging)
# Offset calculation
In the `paging.c` code, in the function `paging_new_4gb`, we find a formula alongside an initialization statement:
`entry[b] = (offset + (b * PAGING_PAGE_SIZE))`
The formula helps us keep track of our entries. For example: if our usable memory returned by the `directory` `kzalloc` call starts at `0x01000000`, this would be:
`0 + (0 * 4096)` or 0. In the second loop, it would be `0 + (1 * 4096)` or `0x01001000` or 4096 bytes ahead, which is around 4 kilobytes. This will go on and on, and it's because each page table is the physical representation of 4096 bytes of physical memory.

After finishing the for loop, the `entry` array is placed at position `0` of the `directory` array and also increments the `offset` by `(PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE)`.
`(PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE)`
`1024 * 4096 = 4194304` or `0x00400000`. 4194304 is 4 megabytes, which is the size of the page directory (look above). 

So we know that:

The Page Directory (PD) is an array that contains 1024 entries, each of them being 4 bytes; and the Page Table (PT) is also an array of 1024 , each entry is 4 bytes and it represents a 4 kilobyte (4096 bytes) section of physical memory. For example:

PD[0] has entry[0] that points to `0x01000000`. PD[0] has entry[1] that points to `0x01001000`, and so on and so forth.
But the next PD...
PD[1] has entry[0] that points to `0x01400000`. PD[1] has entry[1] that points to `0x01401000`, and so on and so forth.

This is possible because we calcualte different `offset` for each page directory. And the offset moves in `n * 4MB`. Meaning that after every pass of the `i` `for` loop, the offset will be incremented around four megabytes. And then each entry is populated with a 4KB physical memory section, until we hit four gigabytes, or `PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE`.

# But why `PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE`?
Taking into consideration we have two `for` loops that will stop at `PAGING_TOTAL_ENTRIES_PER_TABLE`, we know that we're running the `for` loop `PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_TOTAL_ENTRIES_PER_TABLE`, because we have to iterate over every one of them. But, where does `PAGING_PAGE_SIZE` come into the equation?

We consider `PAGING_PAGE_SIZE` because remember: each entry in the Page Table is 4 kilobytes, and `PAGING_PAGE_SIZE` is 4096, or four kilobytes. So, with that in mind, we know that we'll indeed populate 4 gigabytes of memory.
# Four gigabytes of memory?!
Yes. `PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE` or `1024 * 1024 * 4096` equals 4.294.967.296, or four gigabytes :)
# Switching and modifying the PD and PT.
To be able to switch and move around the pages, we need a function that takes the virtual address and calculates the PD and PT from it. For that, we use the following formulas:

directory_index = `aligned_virtual_address / (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE)`
table_index = `aligned_virtual_address % (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE) / PAGING_PAGE_SIZE)`

With `aligned_virtual_address` I'm referring to a virtual address whose modulus by `PAGING_PAGE_SIZE` is equal to zero. That is, `virtual_addr % PAGING_PAGE_SIZE = 0`. If this virtual address isn't aligned, then it's not a valid argument and we cannot get a precise calculation of the PD index or PT index.

let's use them in a few examples. let's start by calculating the PD index.
virtual_address = `0x405000`
pd_index = `0x405000 / (1024 * 4096)`
pd_index = `4,214,784 / (1024 * 4096)`
pd_index = `4,214,784 / (4,194,304)`
pd_index = `1`

let's try the same formula with another virtual address. `0x504000`
pd_index = `0x504000 / (1024*4096)`
pd_index = `5,259,264 / (1024 * 4096)`
pd_index = `5,259,264 / 4,194,304`
pd_index = `1`
*\*Note: we aren't using floating numbers. Just natural numbers.*

nice! now let's go for a PT index. let's use the previous virtual address `0x405000`
pt_index = `0x405000 % (1024 * 4096) / 4096`
pt_index = `4,214,784 % (1024 * 4096) / 4096`
pt_index = `4,214,784 % (4,194,304) / 4096`
pt_index = `20,480 / 4096`
pt_index = `5`

let's do the same as before and now try the formula with the value `0x504000`.
pt_index = `0x504000 % (1024*4096) / 4096`
pt_index = `5,259,264 % (4,194,304) / 4096`
pt_index = `5,259,264 % 4,194,304 / 4096`
pt_index = `1,064,960 / 4096`
pt_index = `260`

and done! I hope this few examples help everyone reading this page to better understand the calculations in our code.
# (uint32_t)(entry & 0xFFFFF000)???
This is an `AND` bitwise operation in which we'll access the first 20 bits of the entry page table entry. Let's visualize it.
`entry` is a 32 bit value, so:
`1111 1111 1111 1111 1111 1111 0011 0110`. This is just an example, the actual entry won't look like this. 
`0xFFFFF000` would look like this:
`1111 1111 1111 1111 1111 1111 0000 0000`. So, when we do the `AND` operation, which expects both bits to be `0` or `1`:

```
1111 1111 1111 1111 1111 1111 0011 0110 &
1111 1111 1111 1111 1111 1111 0000 0000
=
1111 1111 1111 1111 1111 1111 0000 0000
```
And so, `entry & 0xFFFFF000` would be `1111 1111 1111 1111 1111 1111 0000 0000`. Pretty neat!
