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

// E1000 Registers, defined as offset words from base
#define E1000_STATUS          (0x00008 / 4) // Status register
// TX Registers
#define E1000_TDBAL           (0x03800 / 4)  // TX Descriptor Base Address Low - RW
#define E1000_TDBAH           (0x03804 / 4)  // TX Descriptor Base Address High - RW
#define E1000_TDLEN           (0x03808 / 4)  // TX Descriptor Length - RW
#define E1000_TDH             (0x03810 / 4)  // TX Descriptor Head - RW
#define E1000_TDT             (0x03818 / 4)  // TX Descripotr Tail - RW
#define E1000_TCTL            (0x00400 / 4)  // TX Control - RW
#define E1000_TIPG            (0x00410 / 4)  // TX Inter Packet Gap Register

// RX Registers
#define E1000_RCTL     				(0x00100 / 4) // RX Control Register


// Enums
#define E1000_STATUS_UNINIT   0x80080783    // uninitialized status
// TCTL Register enums
#define E1000_TCTL_EN         0x00000002    // enable tx
#define E1000_TCTL_PSP        0x00000008    // pad short packets
#define E1000_TCTL_CT         (0x10 << 12)    // collision threshold
// #define E1000_TCTL_CT         0x00000ff0    // collision threshold
#define E1000_TCTL_COLD       0x003ff000    // collision distance
// RCTL Register enums
#define E1000_RCTL_EN         0x00000002		// enable RX


#define E1000_TX_STS_DD       0x00000001    // DD packet status
#define E1000_TX_CMD_RS      	0x00000008    // Report Status
#define E1000_TX_CMD_EOP     	0x00000001    // End of Packet

volatile uint32_t *e1000_mmio;

struct tx_desc
{
	volatile uint64_t bufaddr;
  // volatile uint32_t bufaddr_63_32;
	volatile uint16_t length; // max: 16288 bytes
	volatile uint8_t cso;
  union {
    struct {
      volatile uint8_t eop : 1;
      volatile uint8_t ifcs : 1;
      volatile uint8_t ic : 1;
      volatile uint8_t rs : 1;
      volatile uint8_t rps : 1;
      volatile uint8_t dext : 1;
      volatile uint8_t vle : 1;
      volatile uint8_t ide : 1;
    } __attribute__((__packed__));
    volatile uint8_t cmd;
  };
	union {
		struct {
			unsigned dd : 1;
			unsigned ec : 1;
			unsigned lc : 1;
			unsigned rsv : 1;
			unsigned padding : 4;
		} __attribute__((__packed__));
		uint8_t status;
	};
	volatile uint8_t css;
	volatile uint16_t special;
} __attribute__((__packed__));

typedef struct tx_desc tx_desc_t;

struct rcv_desc
{
	uint64_t bufaddr;
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
    } __attribute__((__packed__));
    uint8_t status;
};
	uint8_t errors;
	uint16_t special;
} __attribute__((__packed__));

typedef struct rcv_desc rcv_desc_t;

typedef struct __attribute__((packed)) tx_pkt
{
	uint8_t buf[E1000_TX_PKT_SIZE];
} tx_pkt_t;

typedef struct __attribute__((packed)) rcv_pkt
{
	uint8_t buf[E1000_RCV_PKT_SIZE];
} rcv_pkt_t;

struct tctl_reg {
	union {
		struct {
			uint8_t reserved1 : 1;
			uint8_t en : 1;
			uint8_t reserved2 : 1;
			uint8_t psp : 1;
			uint8_t ct : 8;
			uint16_t cold : 10;
			uint8_t swxoff : 1;
			uint8_t reserved3 : 1;
			uint8_t rtlc : 1;
			uint8_t nrtu : 1;
			uint8_t reserved4 : 6;
		};
		uint32_t val;
	};
} __attribute__((packed));

typedef struct tctl_reg tctl_reg_t;

int32_t e1000_init();

int32_t e1000_init_txctl();

int32_t e1000_init_tipg();

int32_t e1000_init_rx();

int32_t e1000_init_rxctl();

int32_t e1000_transmit(uint8_t* data, uint32_t length);

#endif
