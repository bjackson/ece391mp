#ifndef _E1000_H
#define _E1000_H

#include "pci.h"

#define E1000_BASE            (0xfebc0000)

#define E1000_BAR0            (E1000_BASE)
#define E1000_BAR1_PORT       (0xc03f)
#define E1000_BAR6            (0x0003fffe)

// Sizes
#define E1000_DESC_SIZE       64
#define E1000_TX_PKT_SIZE     1518
#define E1000_RCV_PKT_SIZE    2048

// Eß1000 Registers, defined as offset words from base
#define E1000_STATUS          (0x8 / 4) // Status register
#define E1000_TDBAL           (0x03800 / 4)  // TX Descriptor Base Address Low - RW
#define E1000_TDBAH           (0x03804 / 4)  // TX Descriptor Base Address High - RW
#define E1000_TDLEN           (0x03808 / 4)  // TX Descriptor Length - RW
#define E1000_TDH             (0x03810 / 4)  // TX Descriptor Head - RW
#define E1000_TDT             (0x03818 / 4)  // TX Descripotr Tail - RW
#define E1000_TCTL            (0x00400 / 4)  // TX Control - RW
#define E1000_TIPG            (0x00410 / 4)  // TX Inter Packet Gap Register

// Enums
#define E1000_STATUS_UNINIT   0x80080783    // uninitialized status
#define E1000_TCTL_EN         0x00000002    // enable tx
#define E1000_TCTL_PSP        0x00000008    // pad short packets
#define E1000_TCTL_CT         0x00000ff0    // collision threshold
#define E1000_TCTL_COLD       0x003ff000    // collision distance
#define E1000_TX_STS_DD       0x00000001    // DD packet status
#define E1000_TX_CMD_RS      0x00000008    // Report Status
#define E1000_TX_CMD_EOP     0x00000001    // End of Packet

volatile uint32_t *e1000_mmio;

typedef struct __attribute__((packed)) tx_desc
{
	uint32_t bufaddr;
  uint32_t bufaddr_63_32;
	uint16_t length; // max: 16288 bytes
	uint8_t cso;
  union {
    struct {
      uint8_t eop : 1;
      uint8_t ifcs : 1;
      uint8_t ic : 1;
      uint8_t rs : 1;
      uint8_t rps : 1;
      uint8_t dext : 1;
      uint8_t vle : 1;
      uint8_t ide : 1;
    };
    uint8_t cmd;
  };
	uint8_t status;
	uint8_t css;
	uint16_t special;
} tx_desc_t;

typedef struct __attribute__((packed)) rcv_desc
{
	uint32_t bufaddr;
  uint32_t bufaddr_63_32;
	uint16_t length;
	uint16_t checksum;
  union {
    struct {
        uint8_t dd : 1;
        uint8_t eop : 1;
        uint8_t ixsm : 1;
        uint8_t vp : 1;
        uint8_t rsv : 1;
        uint8_t tcpcs : 1;
        uint8_t ipcs : 1;
        uint8_t pif : 1;
    };
    uint8_t status;
};
	uint8_t errors;
	uint16_t special;
} rcv_desc_t;

typedef struct __attribute__((packed)) tx_pkt
{
	uint8_t buf[E1000_TX_PKT_SIZE];
} tx_pkt_t;

typedef struct __attribute__((packed)) rcv_pkt
{
	uint8_t buf[E1000_RCV_PKT_SIZE];
} rcv_pkt_t;

int32_t e1000_init();

int32_t e1000_transmit(uint8_t* data, uint32_t length);

#endif
