/**
 * paging.h
 */
#ifndef PAGING_H
#define PAGING_H

#include "types.h"
#include "lib.h"

#define MAX_ENTRIES 1024

#define FOUR_KB (4 * 1024)
#define FOUR_MB (4 * 1024 * 1024)

#define ACCESS_ALL 1
#define ACCESS_SUPER 0

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

// Map a large (4MB) page
void map_large_page(void* phys, void* virt, uint8_t access);

// Register a page directory entry for a 4KB page table
void register_page_table(uint32_t index, uint32_t* page_table, uint8_t access);

#endif /* PAGING_H */
