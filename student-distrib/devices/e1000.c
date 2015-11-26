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
    memset(tx_packets[i].buf,  0x00, E1000_TX_PKT_SIZE);
    memset(rcv_packets[i].buf, 0x00, E1000_TX_PKT_SIZE);

    tx_descriptors[i].bufaddr  = k_virt_to_phys(tx_packets[i].buf);
    // tx_descriptors[i].bufaddr_63_32 = 0;

    rcv_descriptors[i].bufaddr = k_virt_to_phys(rcv_packets[i].buf);
    rcv_descriptors[i].bufaddr_63_32 = 0;

    tx_descriptors[i].dd = 1;
    tx_descriptors[i].rs = 1;
  }

  debug("&tx_buff: 0x%x\n", k_virt_to_phys(tx_packets[0].buf));


  // Initialize TX descriptor registers
  e1000_mmio[E1000_TDBAL] = k_virt_to_phys(tx_descriptors);
  e1000_mmio[E1000_TDBAH] = 0x00;


  // Set head and tail (TX)
  e1000_mmio[E1000_TDH] = 0x00;
  e1000_mmio[E1000_TDT] = 0x00;
  e1000_mmio[E1000_TDLEN] = sizeof(tx_desc_t) * E1000_DESC_SIZE;

  debug("TDLEN: %d\n", e1000_mmio[E1000_TDLEN]);

  // Init TXCTL register
  e1000_init_txctl();

  // Initialize TXIPG Register
  e1000_init_tipg();

  // Initialize RX registers
  e1000_init_rx();


  return 0;
}

int32_t e1000_transmit(uint8_t* data, uint32_t length) {
  assert_do(length < E1000_TX_PKT_SIZE, {
    return -1;
  });

  uint32_t tdt_idx = e1000_mmio[E1000_TDT];

  // Ensure that queue is not full
  if (tx_descriptors[tdt_idx].dd == 1) {
    tx_desc_t *desc = &tx_descriptors[tdt_idx];
    tx_pkt_t *pkt = &tx_packets[tdt_idx];

    // Clear packet buffer
    memset(pkt->buf, 0x00, E1000_TX_PKT_SIZE);

    memcpy(pkt->buf, data, length);

    // desc->bufaddr = (uint64_t)k_virt_to_phys(pkt->buf);
    desc->length = length;

    debug("tdt: %d, &tx_buff: 0x%x\n", tdt_idx, k_virt_to_phys(pkt->buf));
    debug("tx_buff: %s\n", pkt->buf);
    debug("&desc: 0x%x\n", k_virt_to_phys(desc));

    desc->status  = 0;
    desc->dd      = 0;   // Set descriptor as in use
    desc->rs      = 1;
    desc->eop     = 1;  // End of packet
    desc->special = 0;

    // Update TDT register, which tells e1000
    // that a new packet is available to transmit
    tdt_idx = (tdt_idx + 1) % E1000_DESC_SIZE;
    e1000_mmio[E1000_TDT] = tdt_idx;
  } else {
    debug("E1000 transmit queue is full.\n");
    return -1;
  }

  return 0;
}

int32_t e1000_init_tipg() {
  uint32_t tipg = 0;
  tipg  =  0x00;        // Reserved
  tipg |= (0x06) << 20; // IPGR2
  tipg |= (0x08) << 10; // IPGR1
  tipg |=  0x0A;        // IPGT

  e1000_mmio[E1000_TIPG] = tipg;

  return 0;
}

int32_t e1000_init_txctl() {
  // uint32_t tctl = 0;
  // tctl |= E1000_TCTL_EN;
  // tctl |= E1000_TCTL_PSP;
  // tctl &= ~E1000_TCTL_CT;
  // tctl |= (0x10) << 4;
  // tctl &= ~E1000_TCTL_COLD;
  // tctl |= (0x40) << 12;
  tctl_reg_t tctl;
  tctl.val = 0;
  tctl.en = 1;
  tctl.psp = 1;
  tctl.ct = 0x10;
  tctl.cold = 0x40;

  e1000_mmio[E1000_TCTL] = tctl.val;

  return 0;
}

int32_t e1000_init_rx() {
  e1000_init_rxctl();

  return 0;
}

int32_t e1000_init_rxctl() {
  // For the time being, disable RX.
  uint32_t rctl = 0;

  rctl |= E1000_RCTL_EN; // Enable RX

  e1000_mmio[E1000_RCTL] = rctl;

  return 0;
}
