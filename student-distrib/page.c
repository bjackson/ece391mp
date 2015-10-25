//
//  page.c
//  
//
//  Created by Kevin Lehtiniitty on 10/24/15.
//
//

#include "page.h"

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