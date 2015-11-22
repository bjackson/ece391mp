#include "e1000.h"
#include "../paging.h"

tx_desc_t tx_descriptors[E1000_DESC_SIZE] __attribute__ ((aligned (16)));
tx_pkt_t tx_packets[E1000_DESC_SIZE];

rcv_desc_t rcv_descriptors[E1000_DESC_SIZE] __attribute__ ((aligned (16)));
rcv_pkt_t rcv_packets[E1000_DESC_SIZE];

int32_t e1000_init() {
  //Clear descriptors and packets
  memset(tx_descriptors,  0x00, sizeof(tx_desc_t)  * E1000_DESC_SIZE);
  memset(rcv_descriptors, 0x00, sizeof(rcv_desc_t) * E1000_DESC_SIZE);
  memset(tx_packets,      0x00, sizeof(tx_pkt_t)   * E1000_DESC_SIZE);
  memset(rcv_packets,     0x00, sizeof(rcv_pkt_t)  * E1000_DESC_SIZE);

  e1000_mmio = (uint32_t *)E1000_BASE;

  assert(e1000_mmio[E1000_STATUS] == E1000_STATUS_UNINIT);
  // tx_packets[0].buf[0] = 0x19;
  // printf("VIRT: 0x%x\n", k_virt_to_phys(&tx_packets[0].buf));
  // printf("PHYS: 0x%x\n", &tx_packets[0].buf);

  // Initialize descriptors
  uint32_t i;
  for (i = 0; i < E1000_DESC_SIZE; i++) {
    tx_descriptors[i].bufaddr = k_virt_to_phys(tx_packets[i].buf);
    rcv_descriptors[i].bufaddr = k_virt_to_phys(rcv_packets[i].buf);
    tx_descriptors[i].status |= E1000_TX_STS_DD;
  }

  // Initialize TX descriptor registers
  e1000_mmio[E1000_TDBAH] = k_virt_to_phys(tx_descriptors);
  e1000_mmio[E1000_TDBAL] = 0x00;
  e1000_mmio[E1000_TDLEN] = sizeof(tx_desc_t) * E1000_DESC_SIZE;

  // Set head and tail (TX)
  e1000_mmio[E1000_TDH] = 0x00;
  e1000_mmio[E1000_TDT] = 0x00;

  // Initialize TX Control Register
  e1000_mmio[E1000_TCTL] = 0;
  e1000_mmio[E1000_TCTL] |= E1000_TCTL_EN;
  e1000_mmio[E1000_TCTL] |= E1000_TCTL_PSP;
  e1000_mmio[E1000_TCTL] &= ~E1000_TCTL_CT;
  e1000_mmio[E1000_TCTL] |= (0x10) << 4;
  e1000_mmio[E1000_TCTL] &= ~E1000_TCTL_COLD;
  e1000_mmio[E1000_TCTL] |= (0x40) << 12;

  // Initialize TX IPG Register
  e1000_mmio[E1000_TIPG]  =  0x00;        // Reserved
  e1000_mmio[E1000_TIPG] |= (0x06) << 20; // IPGR2
  e1000_mmio[E1000_TIPG] |= (0x04) << 10; // IPGR1
  e1000_mmio[E1000_TIPG] |=  0x0A;        // IPGT


  return 0;
}

int32_t e1000_transmit(uint8_t data, uint32_t length) {
  assert_do(length < E1000_TX_PKT_SIZE, {
    return -1;
  });

  return 0;
}
