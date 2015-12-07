#include "e1000.h"
#include "../paging.h"
#include "../lib.h"

volatile tx_desc_t tx_descriptors[E1000_DESC_SIZE] __attribute__ ((aligned (16)));
volatile tx_pkt_t tx_packets[E1000_DESC_SIZE];

volatile rx_desc_t rx_descriptors[E1000_DESC_SIZE] __attribute__ ((aligned (16)));
volatile rx_pkt_t rx_packets[E1000_DESC_SIZE];

/*
 * int32_t e1000_init()
 * Description: initializes the e1000 networking controller
 * Inputs: none
 * Outputs: 0 on success
 */
int32_t e1000_init() {
  // *tx_packets = (tx_pkt_t *)E1000_PKT_LOC;
  // *rx_packets = (rx_pkt_t *)(E1000_PKT_LOC + (sizeof(tx_pkt_t) * E1000_DESC_SIZE));
  //Clear descriptors and packets
  memset((void *)tx_descriptors,  0x00, sizeof(tx_desc_t) * E1000_DESC_SIZE);
  memset((void *)rx_descriptors,  0x00, sizeof(rx_desc_t) * E1000_DESC_SIZE);
  memset((void *)tx_packets,      0x00, sizeof(tx_pkt_t)  * E1000_DESC_SIZE);
  memset((void *)rx_packets,      0x00, sizeof(rx_pkt_t)  * E1000_DESC_SIZE);

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

    tx_descriptors[i].bufaddr  = k_virt_to_phys((void *)tx_packets[i].buf);
    rx_descriptors[i].bufaddr = k_virt_to_phys((void *)rx_packets[i].buf);

    tx_descriptors[i].dd = 1;
    tx_descriptors[i].rs = 1;
  }



  // Initialize TX descriptor registers
  e1000_mmio[E1000_TDBAL] = k_virt_to_phys((void *)tx_descriptors);
  e1000_mmio[E1000_TDBAH] = 0x00;

  debug("E1000_TDBAL: 0x%x\n", e1000_mmio[E1000_TDBAL]);


  // Set head and tail (TX)
  e1000_mmio[E1000_TDLEN] = sizeof(tx_desc_t) * E1000_DESC_SIZE;
  e1000_mmio[E1000_TDH] = 0x00;
  e1000_mmio[E1000_TDT] = 0x00;

  debug("TDLEN: %d\n", e1000_mmio[E1000_TDLEN]);

  // Init TXCTL register
  e1000_init_txctl();

  // Initialize TXIPG Register
  e1000_init_tipg();

  // Initialize RX registers
  e1000_init_rx();


  return 0;
}

/* 
 * int32_t e1000_transmit(uint8_t data, uint32_t length)
 * Decsription: Transmits e1000 packets
 * Inputs: data - the data to be transmitted, length - the length of the transmission
 * Outputs: -1 on failure, length on success
 */
int32_t e1000_transmit(uint8_t* data, uint32_t length) {
  assert_do(length < E1000_TX_PKT_SIZE, {
    return -1;
  });

  uint32_t tdt_idx = e1000_mmio[E1000_TDT];

  // Ensure that queue is not full
  if (tx_descriptors[tdt_idx].dd == 1) {
    tx_desc_t *desc = (void *)&tx_descriptors[tdt_idx];
    tx_pkt_t *pkt = (void *)&tx_packets[tdt_idx];

    // Clear packet buffer
    memset(pkt->buf, 0x00, E1000_TX_PKT_SIZE);

    memcpy(pkt->buf, data, length);

    // desc->bufaddr = (uint64_t)k_virt_to_phys(pkt->buf);
    desc->length = length;

    // debug("tdt: %d, &tx_buff: 0x%x\n", tdt_idx, k_virt_to_phys(pkt->buf));
    // debug("tx_buff: %s\n", pkt->buf);
    // debug("&desc: 0x%x\n", k_virt_to_phys(desc));

    // desc->status  = 0;
    desc->dd      = 0;   // Set descriptor as in use
    desc->cso     = 0;
    // desc->rs      = 1;
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

  return length;
}

/*
 * int32_t e1000_recieve(uint8_t data, uint32_t length)
 * Decsription: Recieves e1000 packets
 * Inputs: data - the data to be transmitted, length - the length of the transmission
 * Outputs: -1 on failure, length on success
 */
int32_t e1000_receive(uint8_t* data, uint32_t length) {
  assert_do(length != 0, {
    return -1;
  });

  uint32_t rdt_idx = e1000_mmio[E1000_RDT];

  if (rx_descriptors[rdt_idx].dd == 1) {
    rx_desc_t *desc = (void*)&rx_descriptors[rdt_idx];
    rx_pkt_t *pkt = (void*)&rx_packets[rdt_idx];

    uint32_t pkt_length = desc->length;
    if (pkt_length > length) {
      return -1;
    }

    memcpy(data, pkt->buf, pkt_length);

    desc->dd = 0;
    desc->eop = 0;

    rdt_idx = (rdt_idx + 1) % E1000_DESC_SIZE;

    e1000_mmio[E1000_RDT] = rdt_idx;
    return pkt_length;
  }

  return 0;
}

/*
 * int32_t e1000_init_tipg()
 * Decsription: Initializes the e1000 tpig
 * Input: none
 * Outputs: 0 on success
 */
int32_t e1000_init_tipg() {
  uint32_t tipg = 0x00;
  tipg  =  0x00;        // Reserved
  tipg |= (0x06) << 20; // IPGR2
  tipg |= (0x04) << 10; // IPGR1
  tipg |=  0x0A;        // IPGT

  e1000_mmio[E1000_TIPG] = tipg;

  return 0;
}

/*
 * int32_t e1000_init_txctl()
 * Decsription: Initializes the e1000 tctl
 * Inputs: none
 * Outputs: 0 on success
 */
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

/*
 * int32_t e1000_init_rx()
 * Decsription: Initializes the e1000 rx descriptors
 * Inputs: none
 * Outputs: 0 on success
 */
int32_t e1000_init_rx() {

  initialize_MAC_address();

  // Initialize RX descriptor registers
  e1000_mmio[E1000_RDBAL] = k_virt_to_phys((void *)rx_descriptors);
  e1000_mmio[E1000_RDBAH] = 0x00;

  debug("E1000_RDBAL: 0x%x\n", e1000_mmio[E1000_RDBAL]);


  // Set head and tail (RX)
  e1000_mmio[E1000_RDLEN] = sizeof(rx_desc_t) * E1000_DESC_SIZE;
  e1000_mmio[E1000_RDH] = 0x00;
  e1000_mmio[E1000_RDT] = 0x00;

  e1000_init_rxctl();


  return 0;
}

/*
 * int32_t e1000_init_rxctl()
 * Decsription: Initializes the e1000 rctl
 * Inputs: none
 * Outputs: 0 on success
 */
int32_t e1000_init_rxctl() {
  // For the time being, disable RX.
  rctl_reg_t rctl;
  rctl.val = 0;

  rctl.en = 1; // Enable RX
  rctl.lpe = 1; // Allow long packets
  rctl.lbm = 0; // Turn off loopback-mode
  rctl.rdmts = 2; // INT at 1/8 of RDLEN
  rctl.mo = 0;
  rctl.bam = 1; // Allow to accept broadcast packets.
  rctl.bsize = 0; // 2048 byte size packets

  e1000_mmio[E1000_RCTL] = rctl.val;

  return 0;
}

/*
 * int32_t e1000_read_from_eeprom(unit32_t read_register)
 * Decsription: Reads eeprom
 * Inputs: read_register - register to read from
 * Outputs: 16 highest bits of data
 */
uint16_t e1000_read_from_eeprom(uint32_t read_register) {
  //  31-16  15-8  7-5   4   3-1    0
  //  Data Address RSV. DONE RSV. START
  e1000_mmio[E1000_EERD] = read_register << 8; // Address in bits 8-15
	e1000_mmio[E1000_EERD] |= (E1000_EERD_START);

	while (!(e1000_mmio[E1000_EERD] & E1000_EERD_DONE)) // Wait till finished reading
  ;

  return (uint16_t)(e1000_mmio[E1000_EERD] >> 16); // Data is in high 16 bits
}

/*
 * int32_t initialize_MAC_address()
 * Decsription: Initializes the MAC address
 * Inputs: none
 * Outputs: 0 on success
 */
int32_t initialize_MAC_address() {
  // Initialize MAC Address
  uint32_t ral = 0;
  uint32_t rah = 0;

  ral  = e1000_read_from_eeprom(E1000_EERD_EADDR_12); // Bytes 1-2
  ral |= (uint32_t)(e1000_read_from_eeprom(E1000_EERD_EADDR_34)) << 16; // Bytes 3-4
  rah  = e1000_read_from_eeprom(E1000_EERD_EADDR_56); // Bytes 5-6

  e1000_mmio[E1000_RAL] = ral;
  e1000_mmio[E1000_RAH] = rah;

  debug("E1000 MAC: %x %x\n", ral, rah);

  return 0;
}
