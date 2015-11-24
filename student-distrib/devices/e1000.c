#include "e1000.h"
#include "../paging.h"
#include "../lib.h"

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

  debug("e1000_mmio[E1000_STATUS]: 0x%x\n", e1000_mmio[E1000_STATUS]);

  assert_do(e1000_mmio[E1000_STATUS] == E1000_STATUS_UNINIT, {
    printf("e1000_mmio[E1000_STATUS]: 0x%x\n", e1000_mmio[E1000_STATUS]);
  });
  // tx_packets[0].buf[0] = 0x19;
  // printf("VIRT: 0x%x\n", k_virt_to_phys(&tx_packets[0].buf));
  // printf("PHYS: 0x%x\n", &tx_packets[0].buf);

  // Initialize descriptors
  uint32_t i;
  for (i = 0; i < E1000_DESC_SIZE; i++) {
    tx_descriptors[i].bufaddr  = k_virt_to_phys(tx_packets[i].buf);
    tx_descriptors[i].bufaddr_63_32 = 0;
    rcv_descriptors[i].bufaddr = k_virt_to_phys(rcv_packets[i].buf);
    rcv_descriptors[i].bufaddr_63_32 = 0;
    tx_descriptors[i].status  |= E1000_TX_STS_DD;
  }

  debug("&tx_buff: 0x%x\n", k_virt_to_phys(tx_packets[0].buf));


  // Initialize TX descriptor registers
  e1000_mmio[E1000_TDBAL] = k_virt_to_phys(tx_descriptors);
  e1000_mmio[E1000_TDBAH] = 0x00;
  e1000_mmio[E1000_TDLEN] = sizeof(tx_desc_t) * E1000_DESC_SIZE;

  // Set head and tail (TX)
  e1000_mmio[E1000_TDH] = 0x00;
  e1000_mmio[E1000_TDT] = 0x00;

  // Initialize TX Control Register
  // e1000_mmio[E1000_TCTL]  = 0;
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

int32_t e1000_transmit(uint8_t* data, uint32_t length) {
  assert_do(length < E1000_TX_PKT_SIZE, {
    return -1;
  });

  uint32_t tdt_idx = e1000_mmio[E1000_TDT];

  // Ensure that queue is not full
  if (tx_descriptors[tdt_idx].status & E1000_TX_STS_DD) {
    memset(tx_packets[tdt_idx].buf, 0x00, E1000_TX_PKT_SIZE);
    memcpy(tx_packets[tdt_idx].buf, data, length);

    debug("tdt: %d, &tx_buff: 0x%x\n", tdt_idx, k_virt_to_phys(tx_packets[tdt_idx].buf));
    debug("tx_buff: %s\n", tx_packets[tdt_idx].buf);

    tx_descriptors[tdt_idx].length = length;

    tx_descriptors[tdt_idx].status &= ~E1000_TX_STS_DD;   // Set descriptor as sent
    tx_descriptors[tdt_idx].cmd    |= E1000_TX_CMD_RS;
    tx_descriptors[tdt_idx].cmd    |= E1000_TX_CMD_EOP;  // End of packet

    e1000_mmio[E1000_TDT] = (tdt_idx + 1) % E1000_DESC_SIZE; // Cycle through packets
  } else {
    debug("E1000 transmit queue is full.");
    return -1;
  }

  return 0;
}
