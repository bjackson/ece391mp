/**
 * paging.c
 *
 * vim:ts=4 expandtab
 */
#include "paging.h"

uint32_t page_dirs[MAX_TASKS][MAX_ENTRIES] __attribute__((aligned(FOUR_KB)));
uint32_t page_tables[MAX_TASKS][MAX_ENTRIES] __attribute__((aligned(FOUR_KB)));

/**
 *
 */
void init_paging() {
    memset(page_dirs, 0x00, sizeof(uint32_t) * MAX_ENTRIES * MAX_TASKS);
    memset(page_tables, 0x00, sizeof(uint32_t) * MAX_ENTRIES * MAX_TASKS);

    // Kernel page table
    register_page_table(page_dirs[KERNEL_PID], 0, page_tables[KERNEL_PID], ACCESS_SUPER);

    // Map page for video memory in kernel page table
    map_page(page_tables[KERNEL_PID], ((void*) VIDEO), ((void*) VIDEO), ACCESS_ALL);

    // Map large page for kernel code
    map_large_page(page_dirs[KERNEL_PID], ((void*) FOUR_MB), ((void*) FOUR_MB),
            ACCESS_SUPER, GLOBAL, CACHE_ENABLED, WRITE_THROUGH_ENABLED);

    /*
    // Map E1000 network card into memory
    mmap(page_dirs[KERNEL_PID], ((void*) E1000_BASE),
            ((void*) E1000_BASE), ACCESS_SUPER, GLOBAL);
    */

    // Enable paging - from OSDev guide at http://wiki.osdev.org/Paging
    asm volatile (
            "movl $page_dirs, %%eax        /* Load paging directory */      ;"
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

    page_table[(((uint32_t) virt) >> 12) & 0x3FF] = pt_entry.val;
}

/**
 *
 */
void map_large_page(uint32_t* page_dir, void* phys, void* virt,
        uint8_t access, uint8_t global, uint8_t cache_disabled, uint8_t write_through) {
    pd_large_entry_t kernel_pd_entry;
    memset(&kernel_pd_entry, 0x00, sizeof(pd_large_entry_t));

    kernel_pd_entry.present = 1;        // Present
    kernel_pd_entry.read_write = 1;     // Read/Write
    kernel_pd_entry.user_supervisor = access;
    kernel_pd_entry.write_through = write_through;  // Write-Through caching enabled
    kernel_pd_entry.cache_disabled = cache_disabled;
    kernel_pd_entry.size = 1;           // 4MB pages
    kernel_pd_entry.global = global;    // If global, don't flush TLB if CR3 is reset
    kernel_pd_entry.addr = ((uint32_t) phys) >> 22;

    page_dir[((uint32_t) virt) >> 22] = kernel_pd_entry.val;
}

// Translates a kernel virtual address to a physical address
// @param virtual virtual address to translate
// @return physical address
/**
 *
 */
uint32_t k_virt_to_phys(void* virtual) {

  uint32_t page_idx = (uint32_t) virtual >> 22;

  assert(page_idx < MAX_TASKS);

  uint32_t offset = (uint32_t) virtual & 0x003FFFFF;

  uint32_t phys_addr = (((pd_large_entry_t) page_dirs[KERNEL_PID][page_idx]).addr << 22) + offset;
  // debug("phys_addr: 0x%x\n", phys_addr);
  return phys_addr;

}

// Translates a virtual address to a physical address
// @param virtual virtual address to translate
// @return physical address
uint32_t virt_to_phys(void* virtual) {
  pcb_t *pcb = get_pcb_ptr();
  uint32_t pid = pcb->pid;

  uint32_t page_idx = (uint32_t) virtual >> 22;

  assert_do(page_idx < MAX_ENTRIES, {
    debug("virtual: 0x%x, page_idx: %d\n", page_idx, virtual);
  });

  uint32_t offset = (uint32_t) virtual & 0x003FFFFF;

  uint32_t phys_addr = (((pd_large_entry_t) page_dirs[pid][page_idx]).addr << 22) + offset;
  // debug("phys_addr: 0x%x\n", phys_addr);
  return phys_addr;

}

/**
 *
 */
void register_page_table(uint32_t* page_dir, uint32_t index,
        uint32_t* page_table, uint8_t access) {
    pd_entry_t pd_entry;
    memset(&pd_entry, 0x00, sizeof(pd_entry_t));

    pd_entry.present = 1;         // Present
    pd_entry.read_write = 1;      // Read/Write
    pd_entry.user_supervisor = access;
    pd_entry.cache_disabled = 0;  // Caching not disabled
    pd_entry.size = 0;            // 4KB pages
    pd_entry.addr = ((uint32_t) page_table) >> 12;

    page_dir[index] = pd_entry.val;
}

/**
 *
 */
void init_task_paging(uint32_t pid) {
    // Register kernel page table
    register_page_table(page_dirs[pid], 0, page_tables[KERNEL_PID], ACCESS_SUPER);

    // Register user page table
    register_page_table(page_dirs[pid], 256, page_tables[pid], ACCESS_ALL);

    // Map large page for kernel code
    map_large_page(page_dirs[pid], ((void*) FOUR_MB), ((void*) FOUR_MB),
            ACCESS_SUPER, GLOBAL, CACHE_ENABLED, WRITE_THROUGH_ENABLED);

    // Map large page for loading user-level program
    map_large_page(page_dirs[pid], ((void*) (FOUR_MB + (pid * FOUR_MB))),
            ((void*) (128 * MB)), ACCESS_ALL, NOT_GLOBAL, CACHE_ENABLED, WRITE_THROUGH_ENABLED);

    // Change CR3 register to new paging directory
    set_page_dir(pid);
}

/**
 *
 */
void set_page_dir(uint32_t pid) {
    // Load CR3 with address of page directory for process with pid
    asm volatile ("movl %0, %%cr3;"::"r"(&(page_dirs[pid])));
}

/**
 *
 */
void restore_parent_paging(uint32_t pid, uint32_t parent_pid) {
    set_page_dir(parent_pid);

    // Clear paging structures of old process
    //memset(page_dirs[pid], 0x00, sizeof(uint32_t) * MAX_ENTRIES);
    //memset(page_tables[pid], 0x00, sizeof(uint32_t) * MAX_ENTRIES);
}

/**
 *
 */
void mmap(void* phys, void* virt, uint8_t access) {
    pcb_t* pcb = get_pcb_ptr();
    map_page(page_tables[(pcb == NULL) ? KERNEL_PID : pcb->pid],
            phys, virt, access);
}

void mmap_large(void* phys, void* virt, uint8_t access, uint8_t write_through) {
    pcb_t* pcb = get_pcb_ptr();
    map_page(page_dirs[(pcb == NULL) ? KERNEL_PID : pcb->pid],
            phys, virt, access);
}
