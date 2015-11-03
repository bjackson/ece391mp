/**
 * paging.c
 *
 * vim:ts=4 expandtab
 */
#include "paging.h"

uint32_t page_directory[MAX_ENTRIES] __attribute__((aligned(FOUR_KB)));
uint32_t kernel_page_table[MAX_ENTRIES] __attribute__((aligned(FOUR_KB)));

/**
 *
 */
void init_paging() {
    memset(page_directory, 0x00, sizeof(uint32_t) * MAX_ENTRIES);
    memset(kernel_page_table, 0x00, sizeof(uint32_t) * MAX_ENTRIES);

    // Kernel page table
    register_page_table(0, kernel_page_table, ACCESS_ALL);

    // Map page for video memory in kernel page table
    map_page(kernel_page_table, VIDEO, VIDEO, ACCESS_ALL);

    // Map large page for kernel code
    map_large_page(FOUR_MB, FOUR_MB, ACCESS_SUPER);

    // Enable paging - from OSDev guide at http://wiki.osdev.org/Paging
    asm volatile (
            "movl $page_directory, %%eax   /* Load paging directory */      ;"
            "movl %%eax, %%cr3                                              ;"

            "movl %%cr4, %%eax             /* Enable PSE */                 ;"
            "orl  $0x00000010, %%eax                                        ;"
            "movl %%eax, %%cr4                                              ;"

            "movl %%cr0, %%eax             /* Set paging bit */             ;"
            "orl  $0x80000000, %%eax                                        ;"
            "movl %%eax, %%cr0                                              ;"
            : : : "eax");
}

/**
 *
 */
void map_page(uint32_t* page_table, void* phys, void* virt, uint8_t access) {
    pt_entry_t pt_entry;
    memset(&pt_entry, 0x00, sizeof(pt_entry_t));

    pt_entry.present = 1;        // Present
    pt_entry.read_write = 1;     // Read/Write
    pt_entry.user_supervisor = access;
    pt_entry.write_through = 1;  // Write-Through caching enabled
    pt_entry.cache_disabled = 0; // Caching not disabled
    pt_entry.global = 0;         // Flush TLB if CR3 is reset
    pt_entry.addr = ((uint32_t) phys) >> 12;

    page_table[((uint32_t) virt) >> 12] = pt_entry.val;
}

/**
 *
 */
void map_large_page(void* phys, void* virt, uint8_t access) {
    pd_large_entry_t kernel_pd_entry;
    memset(&kernel_pd_entry, 0x00, sizeof(pd_entry_t));

    kernel_pd_entry.present = 1;        // Present
    kernel_pd_entry.read_write = 1;     // Read/Write
    kernel_pd_entry.user_supervisor = access;
    kernel_pd_entry.write_through = 1;  // Write-Through caching enabled
    kernel_pd_entry.cache_disabled = 0; // Caching not disabled
    kernel_pd_entry.size = 1;           // 4MB pages
    kernel_pd_entry.global = 0;         // Flush TLB if CR3 is reset
    kernel_pd_entry.addr = ((uint32_t) phys) >> 22;

    page_directory[((uint32_t) virt) >> 22] = kernel_pd_entry.val;
}

/**
 *
 */
void register_page_table(uint32_t index, uint32_t* page_table, uint8_t access) {
    pd_entry_t pd_entry;
    memset(&pd_entry, 0x00, sizeof(pd_entry_t));

    pd_entry.present = 1;         // Present
    pd_entry.read_write = 1;      // Read/Write
    pd_entry.user_supervisor = access;
    pd_entry.cache_disabled = 0;  // Caching not disabled
    pd_entry.size = 0;            // 4KB pages
    pd_entry.addr = ((uint32_t) page_table) >> 12;

    page_directory[index] = pd_entry.val;
}

