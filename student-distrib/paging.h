/**
 * paging.h
 */
#ifndef PAGING_H
#define PAGING_H

#include "types.h"
#include "lib.h"
#include "tasks.h"
#include "devices/e1000.h"
#include "devices/terminal.h"

#define MAX_ENTRIES 1024

/*
 * We only need to allocate two page tables for each task. One will be for
 * memory addresses in the range [0GB,4MB) and the other for memory addresses
 * in the range [1GB, 1GB + 4MB)
 */
#define NUM_PAGE_TABLES 2

#define ACCESS_ALL 1
#define ACCESS_SUPER 0
#define GLOBAL 1
#define NOT_GLOBAL 0
#define CACHE_ENABLED 0
#define CACHE_DISABLED 1
#define WRITE_THROUGH_ENABLED 1
#define WRITE_THROUGH_DISABLED 0

// Struct for 4KB page directory entries
typedef union pd_entry_t {
    uint32_t val;
    struct {
        uint8_t present : 1;
        uint8_t read_write : 1;
        uint8_t user_supervisor : 1;
        uint8_t write_through : 1;
        uint8_t cache_disabled : 1;
        uint8_t accessed : 1;
        uint8_t unused0 : 1;
        uint8_t size : 1;
        uint8_t ignored : 1;
        uint8_t available : 3;
        uint32_t addr : 20;
    } __attribute__((packed));
} pd_entry_t;

// Struct for 4MB page directory entries
typedef union pd_large_entry_t {
    uint32_t val;
    struct {
        uint8_t present : 1;
        uint8_t read_write : 1;
        uint8_t user_supervisor : 1;
        uint8_t write_through : 1;
        uint8_t cache_disabled : 1;
        uint8_t accessed : 1;
        uint8_t dirty : 1;
        uint8_t size : 1;
        uint8_t global : 1;
        uint8_t available : 3;
        uint16_t reserved0 : 10;
        uint32_t addr : 10;
    } __attribute__((packed));
} pd_large_entry_t;

// Struct for page table entries
typedef union pt_entry_t {
    uint32_t val;
    struct {
        uint8_t present : 1;
        uint8_t read_write : 1;
        uint8_t user_supervisor : 1;
        uint8_t write_through : 1;
        uint8_t cache_disabled : 1;
        uint8_t accessed : 1;
        uint8_t dirty : 1;
        uint8_t reserved0 : 1;
        uint8_t global : 1;
        uint8_t available : 3;
        uint32_t addr : 20;
    } __attribute__((packed));
} pt_entry_t;

// Initialize the paging structures
void init_paging();

// Map a small (4KB) page
void map_page(uint32_t* page_table, void* phys, void* virt, uint8_t access);

// Unmap a small (4KB) page
void unmap_page(uint32_t* page_table, void* virt);

// Map a large (4MB) page
void map_large_page(uint32_t* page_dir, void* phys, void* virt,
        uint8_t access, uint8_t global, uint8_t cache_disabled, uint8_t write_through);

// Register a page directory entry for a 4KB page table
void register_page_table(uint32_t* page_dir, uint32_t index,
        uint32_t* page_table, uint8_t access);

//
void init_task_paging(uint32_t pid);

//
void set_page_dir(uint32_t pid);

//
void restore_parent_paging(uint32_t pid, uint32_t parent_pid);

//
void remap_video_memory(uint32_t old_pid, uint32_t new_pid);

//
uint32_t k_virt_to_phys(void* virtual);

//
void mmap(void* phys, void* virt, uint8_t access);

//
void munmap(void* virt);

void mmap_large(void* phys, void* virt, uint8_t access, uint8_t write_through);

#endif /* PAGING_H */
