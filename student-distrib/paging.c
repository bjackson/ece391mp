/**
 * paging.c - <description here>
 *
 * vim:ts=4 expandtab
 */
#include "paging.h"

//
pd_entry_t page_directory[MAX_ENTRIES] __attribute__((aligned(4 * 1024)));
pt_entry_t kernel_page_table[MAX_ENTRIES] __attribute__((aligned(4 * 1024)));

//uint32_t user1_page_table[MAX_ENTRIES] __attribute__((aligned(0x4000)));  //8,000-12,000
//uint32_t user2_page_table[MAX_ENTRIES] __attribute__((aligned(0x4000)));  //12 000-16,000

// function being built as the starting point to do paging. everything after this functions is notes.
/*
 * init_paging
 *      DESCRIPTION: Initializes paging
 *      INPUTS: none
 *      OUTPUTS: none
 *      RETURN VALUE: none
 *      SIDE EFFECTS:
 */
void init_paging() {
    memset(page_directory, 0x00, sizeof(pd_entry_t) * MAX_ENTRIES);
    memset(page_directory, 0x00, sizeof(pt_entry_t) * MAX_ENTRIES);

    //0x00400000;
    //0000000001 0000000000 000000000000

    // 0MB - 4MB
    int i;
    for(i = 0; i < MAX_ENTRIES; i++) {
        pt_entry_t pt_entry;
        memset(&pt_entry, 0x00, sizeof(pt_entry_t));

        pt_entry.present = (i == 0) ? 0 : 1;
        pt_entry.read_write = 1;
        pt_entry.user_supervisor = 0;
        pt_entry.cache_disabled = 1;
        pt_entry.dirty = 0;
        pt_entry.global = 0;
        pt_entry.addr = i * (4 * 1024);
        kernel_page_table[i] = pt_entry;
    }
    pd_entry_t pd_entry;
    memset(&pd_entry, 0x00, sizeof(pd_entry_t));
    pd_entry.present = 1;         // Present
    pd_entry.read_write = 1;      // Read/Write
    pd_entry.user_supervisor = 0; // Supervisor only
    pd_entry.cache_disabled = 1;  // Disable caching
    pd_entry.size = 0;            // 4KB pages
    pd_entry.addr = (uint32_t) kernel_page_table;
    page_directory[0] = pd_entry;

    // 4MB - 8MB
    pd_entry_t kernel_pd_entry;
    memset(&kernel_pd_entry, 0x00, sizeof(pd_entry_t));
    kernel_pd_entry.present = 1;         // Present
    kernel_pd_entry.read_write = 1;      // Read/Write
    kernel_pd_entry.user_supervisor = 0; // Supervisor only
    kernel_pd_entry.cache_disabled = 1;  // Disable caching
    kernel_pd_entry.size = 1;            // 4MB pages
    kernel_pd_entry.addr = 0x400000;
    page_directory[1] = kernel_pd_entry;

    // Enable paging - from OSDev guide at http://wiki.osdev.org/Paging
    asm volatile (
            "movl $page_directory, %%eax   /* Load paging directory */      ;"
            "movl %%eax, %%cr3                                              ;"

            "movl %%cr0, %%eax             /* Set paging bit */             ;"
            "orl  $0x80000000, %%eax                                        ;"
            "movl %%eax, %%cr0                                              ;"

            "movl %%cr4, %%eax             /* Enable PSE */                 ;"
            "orl  $0x00000010, %%eax                                        ;"
            "movl %%eax, %%cr4                                              "
            : : : "eax");
}

/*
 * get_physaddr
 *      DESCRIPTION: gets the physical address that corresponds to a given virtual address
 *      INPUTS: virtualaddr - pointer to the virtual address
 *      OUTPUTS: none
 *      RETURN VALUE: pointer to the physical address
 *      SIDE EFFECTS: none
 *
void * get_physaddr(void * virtualaddr) {
    unsigned long pdindex = (unsigned long)virtualaddr >> 22;
    unsigned long ptindex = (unsigned long)virtualaddr >> 12 & 0x03FF;

    unsigned long * pd = (unsigned long *)0xFFFFF000;
    // Here you need to check whether the PD entry is present.

    unsigned long * pt = ((unsigned long *)0xFFC00000) + (0x400 * pdindex);
    // Here you need to check whether the PT entry is present.

    return (void *)((pt[ptindex] & ~0xFFF) + ((unsigned long)virtualaddr & 0xFFF));
}
 */

/*
 * map_page
 *      DESCRIPTION: map a virtual address to a physical address
 *      INPUTS: physaddr - pointer to the physical address, virtualaddr - pointer to the
 *              virtual address, flags - flags
 *      OUTPUTS: none
 *      RETURN VALUE: none
 *      SIDE EFFECTS: maps the virtual address to a physical address
 *
void map_page(void * physaddr, void * virtualaddr, unsigned int flags) {
    // Make sure that both addresses are page-aligned.

    unsigned long pdindex = (unsigned long)virtualaddr >> 22;
    unsigned long ptindex = (unsigned long)virtualaddr >> 12 & 0x03FF;

    unsigned long * pd = (unsigned long *)0xFFFFF000;
    // Here you need to check whether the PD entry is present.
    // When it is not present, you need to create a new empty PT and
    // adjust the PDE accordingly.

    unsigned long * pt = ((unsigned long *)0xFFC00000) + (0x400 * pdindex);
    // Here you need to check whether the PT entry is present.
    // When it is, then there is already a mapping present. What do you do now?

    pt[ptindex] = ((unsigned long)physaddr) | (flags & 0xFFF) | 0x01; // Present

    // Now you need to flush the entry in the TLB
    // or you might not notice the change.
}
 */

/*
 * intialize the page directory and first page table
void init_one_page() {
    int i;

    // initialize the page directory to empty
    for (i = 0; i < 1024; i++) {
        // This sets the following flags to the pages:
        //   Supervisor: Only kernel-mode can access them
        //   Write Enabled: It can be both read from and written to
        //   Not Present: The page table is not present
        page_directory[i] = 0x00000002;
    }

    // initialize the first page table
    for(i = 0; i < 1024; i++) {
        // As the address is page aligned, it will always leave 12 bits zeroed.
        // Those bits are used by the attributes ;)
        page_table[i] = (i * 0x1000) | 3; // attributes: supervisor level, read/write, present.
    }

    // add the page table to the page directory
    // attributes: supervisor level, read/write, present
    page_directory[0] = ((unsigned int)page_table) | 3;
}
*/

