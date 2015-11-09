/* kernel.c - the C part of the kernel
 * vim:ts=4 expandtab
 */

#include "multiboot.h"
#include "x86_desc.h"
#include "lib.h"
#include "devices/i8259.h"
#include "interrupts/interrupts.h"
#include "paging.h"
#include "devices/rtc.h"
#include "devices/terminal.h"
#include "devices/filesys.h"

/* Macros. */
/* Check if the bit BIT in FLAGS is set. */
#define CHECK_FLAG(flags,bit)   ((flags) & (1 << (bit)))

#define RTC_REGA 0x8A
#define RTC_REGB 0x8B

// Check if MAGIC is valid and print the Multiboot information structure pointed by ADDR.
void entry (unsigned long magic, unsigned long addr) {
    multiboot_info_t *mbi;
    uint32_t fs_start_addr;

    /* Clear the screen. */
    clear();

    /* Am I booted by a Multiboot-compliant boot loader? */
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
    {
        printf ("Invalid magic number: 0x%#x\n", (unsigned) magic);
        return;
    }

    /* Set MBI to the address of the Multiboot information structure. */
    mbi = (multiboot_info_t *) addr;

    /* Print out the flags. */
    printf ("flags = 0x%#x\n", (unsigned) mbi->flags);

    /* Are mem_* valid? */
    if (CHECK_FLAG (mbi->flags, 0))
        printf ("mem_lower = %uKB, mem_upper = %uKB\n",
                (unsigned) mbi->mem_lower, (unsigned) mbi->mem_upper);

    /* Is boot_device valid? */
    if (CHECK_FLAG (mbi->flags, 1))
        printf ("boot_device = 0x%#x\n", (unsigned) mbi->boot_device);

    /* Is the command line passed? */
    if (CHECK_FLAG (mbi->flags, 2))
        printf ("cmdline = %s\n", (char *) mbi->cmdline);

    if (CHECK_FLAG (mbi->flags, 3)) {
        int mod_count = 0;
        int i;
        module_t* mod = (module_t*)mbi->mods_addr;

        while(mod_count < mbi->mods_count) {
            if(mod_count == 0) {
                fs_start_addr = mod->mod_start;
            }

            printf("Module %d loaded at address: 0x%#x\n", mod_count, (unsigned int)mod->mod_start);
            printf("Module %d ends at address: 0x%#x\n", mod_count, (unsigned int)mod->mod_end);
            printf("First few bytes of module:\n");
            for(i = 0; i<16; i++) {
                printf("0x%x ", *((char*)(mod->mod_start+i)));
            }
            printf("\n");
            mod_count++;
            mod++;
        }
    } /* Bits 4 and 5 are mutually exclusive! */
    if (CHECK_FLAG (mbi->flags, 4) && CHECK_FLAG (mbi->flags, 5))
    {
        printf ("Both bits 4 and 5 are set.\n");
        return;
    }

    /* Is the section header table of ELF valid? */
    if (CHECK_FLAG (mbi->flags, 5))
    {
        elf_section_header_table_t *elf_sec = &(mbi->elf_sec);

        printf ("elf_sec: num = %u, size = 0x%#x,"
                " addr = 0x%#x, shndx = 0x%#x\n",
                (unsigned) elf_sec->num, (unsigned) elf_sec->size,
                (unsigned) elf_sec->addr, (unsigned) elf_sec->shndx);
    }

    /* Are mmap_* valid? */
    if (CHECK_FLAG (mbi->flags, 6))
    {
        memory_map_t *mmap;

        printf ("mmap_addr = 0x%#x, mmap_length = 0x%x\n",
                (unsigned) mbi->mmap_addr, (unsigned) mbi->mmap_length);
        for (mmap = (memory_map_t *) mbi->mmap_addr;
                (unsigned long) mmap < mbi->mmap_addr + mbi->mmap_length;
                mmap = (memory_map_t *) ((unsigned long) mmap
                    + mmap->size + sizeof (mmap->size)))
            printf (" size = 0x%x,     base_addr = 0x%#x%#x\n"
                    "     type = 0x%x,  length    = 0x%#x%#x\n",
                    (unsigned) mmap->size,
                    (unsigned) mmap->base_addr_high,
                    (unsigned) mmap->base_addr_low,
                    (unsigned) mmap->type,
                    (unsigned) mmap->length_high,
                    (unsigned) mmap->length_low);
    }

    /* Construct an LDT entry in the GDT */
    {
        seg_desc_t the_ldt_desc;
        the_ldt_desc.granularity    = 0;
        the_ldt_desc.opsize         = 1;
        the_ldt_desc.reserved       = 0;
        the_ldt_desc.avail          = 0;
        the_ldt_desc.present        = 1;
        the_ldt_desc.dpl            = 0x0;
        the_ldt_desc.sys            = 0;
        the_ldt_desc.type           = 0x2;

        SET_LDT_PARAMS(the_ldt_desc, &ldt, ldt_size);
        ldt_desc_ptr = the_ldt_desc;
        lldt(KERNEL_LDT);
    }

    /* Construct a TSS entry in the GDT */
    {
        seg_desc_t the_tss_desc;
        the_tss_desc.granularity    = 0;
        the_tss_desc.opsize         = 0;
        the_tss_desc.reserved       = 0;
        the_tss_desc.avail          = 0;
        the_tss_desc.seg_lim_19_16  = TSS_SIZE & 0x000F0000;
        the_tss_desc.present        = 1;
        the_tss_desc.dpl            = 0x0;
        the_tss_desc.sys            = 0;
        the_tss_desc.type           = 0x9;
        the_tss_desc.seg_lim_15_00  = TSS_SIZE & 0x0000FFFF;

        SET_TSS_PARAMS(the_tss_desc, &tss, tss_size);

        tss_desc_ptr = the_tss_desc;

        tss.ldt_segment_selector = KERNEL_LDT;
        tss.ss0 = KERNEL_DS;
        tss.esp0 = 0x800000;
        ltr(KERNEL_TSS);
    }

    clear(); // Clear video memory
    printf("TrumpOS - Making Computing Great Again\n");
    printf("======================================\n\n");

    /**
     * Initialize devices, memory, filesystem, enable device interrupts on the
     * PIC, any other initialization stuff...
     */
    i8259_init(); // Init PIC
    enable_irq(SLAVE_IRQ); // Enable IRQs 8-15
    enable_irq(KEYBOARD_IRQ); // Enable keyboard interrupt

    init_idt(); // Initialize interrupt handlers

    init_paging(); // Initialize paging

    init_rtc(); // Initialize RTC

    fs_init(fs_start_addr); // Initialize the file system

    terminal_open(NULL); // Initialize the terminal driver

    // Enable interrupts
    sti();

    /**
     * TEST CODE

    fs_test(); // Test the filesystem

    // Test the terminal driver
    int32_t result;
    uint8_t input_buffer[256];
    memset(input_buffer, 0x00, sizeof(input_buffer));
    while((result = terminal_read(0, input_buffer, 256)) > 0) {
        printf("Read %d bytes\n", result);
        printf("Read: %s", input_buffer);
        memset(input_buffer, 0x00, sizeof(input_buffer));
    }

    // Test handling of page faults (uncomment for page fault)
    printf("%s\n", 0xDEADBEEF);
     */

    /* Execute the first program (`shell') ... */

    // Spin (nicely, so we don't chew up cycles)
    asm volatile (".1hlt: hlt; jmp .1hlt;");
}

