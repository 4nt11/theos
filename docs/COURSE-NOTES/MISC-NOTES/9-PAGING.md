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
