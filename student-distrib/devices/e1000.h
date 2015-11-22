#ifndef _E1000_H
#define _E1000_H

#include "pci.h"

#define E1000_BASE            (0xfebc0000)

#define E1000_BAR0            (E1000_BASE)
#define E1000_BAR1_PORT       (0xc03f)
#define E1000_BAR6            (0x0003fffe)

#define E1000_TX_PKT_SIZE     1518
#define E1000_RCV_PKT_SIZE    2048

// E1000 Registers, defined as offset words from base
#define E1000_STATUS          0x8 / 4


#define E1000_STATUS_UNINIT   0x80080783

volatile uint32_t *e1000_mmio;

typedef struct __attribute__((packed)) tx_desc
{
	uint32_t bufaddr;
  uint32_t bufaddr_63_32;
	uint16_t length;
	uint8_t cso;
	uint8_t cmd;
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

struct __attribute__((packed)) tx_pkt
{
	uint8_t buf[E1000_TX_PKT_SIZE];
} tx_pkt_t;

struct __attribute__((packed)) rcv_pkt
{
	uint8_t buf[E1000_RCV_PKT_SIZE];
} rcv_pkt_t;

int32_t e1000_init();

#endif
