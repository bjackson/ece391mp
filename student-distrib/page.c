//
//  page.c
//  
//
//  Created by Kevin Lehtiniitty on 10/24/15.
//
//

#include "page.h"

#define MAX_ENTRIES

/* varaible for page tables. 4096 bytes because 4kb */
uint32_t page_directory[3] __attribute__((aligned(4000)));
uint32_t kernel_page_table[MAX_ENTRIES] __attribute__((aligned(4000))); //4,000-8,000
uint32_t user1_page_table[MAX_ENTRIES] __attribute__((aligned(4000)));  //8,000-12,000
uint32_t user2_page_table[MAX_ENTRIES] __attribute__((aligned(4000)));  //12 000-16,000

// function being built as the starting point to do paging. everything after this functions is notes.
/*
 * init_pages
 *      DESCRIPTION: initializes paging
 *      INPUTS: none
 *      OUTPUTS: none
 *      RETURN VALUE: none
 *      SIDE EFFECTS: clobbers eax
 *
 */
void init_pages() {
    //Enable paging - form OSDev guide at http://wiki.osdev.org/Paging
    asm volatile (
            "movl $page_directory, %%eax      /* enable paging */           ;"
            "movl %%eax, %%cr3                                              ;"
            "movl %%cr0, %%eax                /* set paging bit */          ;"
            "orl $0x80000000, %%eax                                         ;"
            "movl %%eax, %%cr0                                              ;"
            "movl %%cr4, %%eax                  /* enable PSE */            ;"
            "orl $0x00000010, %%eax                                         ;"
            "movl %%eax, %%cr4                                              ;"
            : /* no outputs */
            : /* no inputs */
            : "eax");
    
    int i;
    
    // initialize kernel table
    for (i = 0; i < MAX_ENTRIES; i++) {
        //init page address to array avlue and all settings to 0
        kernel_page_table[i] = i;
        kernel_page_table[1] << 12;
    }
    
    // initialize user space 1 table
    for (i = 0; i < MAX_ENTRIES; i++) {
        //init page address to array avlue and all settings to 0
        user1_page_table[i] = i;
        user1_page_table[1] << 12;
    }
    
    // initialize user 2 table
    for (i = 0; i < MAX_ENTRIES; i++) {
        //init page address to array avlue and all settings to 0
        user2_page_table[i] = i;
        user2_page_table[1] << 12;
    }
    
    // load page tables into the directory
    page_directory[0] = ((unsigned int)kernel_page_table);
    page_directory[1] = ((unsigned int)user1_page_table);
    page_directory[2] = ((unsigned int)user2_page_table);
}









/*
 * get_physaddr
 *      DESCRIPTION: gets the physical address that corresponds to a given virtual address
 *      INPUTS: virtualaddr - pointer to the virtual address
 *      OUTPUTS: none
 *      RETURN VALUE: pointer to the physical address
 *      SIDE EFFECTS: none
 *
 */
void * get_physaddr(void * virtualaddr)
{
    unsigned long pdindex = (unsigned long)virtualaddr >> 22;
    unsigned long ptindex = (unsigned long)virtualaddr >> 12 & 0x03FF;
    
    unsigned long * pd = (unsigned long *)0xFFFFF000;
    // Here you need to check whether the PD entry is present.
    
    unsigned long * pt = ((unsigned long *)0xFFC00000) + (0x400 * pdindex);
    // Here you need to check whether the PT entry is present.
    
    return (void *)((pt[ptindex] & ~0xFFF) + ((unsigned long)virtualaddr & 0xFFF));
}

/*
 * map_page
 *      DESCRIPTION: map a virtual address to a physical address
 *      INPUTS: physaddr - pointer to the physical address, virtualaddr - pointer to the 
 *              virtual address, flags - flags
 *      OUTPUTS: none
 *      RETURN VALUE: none
 *      SIDE EFFECTS: maps the virtual address to a physical address
 *
 */
void map_page(void * physaddr, void * virtualaddr, unsigned int flags)
{
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

// intialize the page directory and first page table
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