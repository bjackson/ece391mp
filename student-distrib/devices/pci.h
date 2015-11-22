#ifndef _PCI_H
#define _PCI_H

#include "../lib.h"
#include "../types.h"

#define PCI_BAR_MMIO 0
#define PCI_BAR_PORT 1

typedef struct pci_device {
    uint8_t device_name[32];
    uint8_t device_version[32];

    uint32_t bar[6];
    uint32_t bar_type[6];
    uint32_t bar_size[6];
    uint8_t irq_line;
} pci_device_t;



#endif
