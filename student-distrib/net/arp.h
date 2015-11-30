#ifndef _ARP_H
#define _ARP_H

#include "../types.h"

//! ARP request operation.
#define ARP_OP_REQUEST          1
//! ARP reply operation.
#define ARP_OP_REPLY            2
//! RARP request operation.
#define RARP_OP_REQUEST         3
//! RARP reply operation.
#define RARP_OP_REPLY           4

// The ARP packet structure.
typedef struct arp
{
        //! Format of hardware address.
        uint16_t arp_hard_type;
        //! Format of protocol address.
        uint16_t arp_proto_type;
        //! Length of hardware address.
        uint8_t  arp_hard_size;
        //! Length of protocol address.
        uint8_t  arp_proto_size;
        //! ARP operation code (command).
        uint16_t arp_op;
        //! Hardware source address.
        uint8_t  arp_eth_source[6];
        //! IP source address.
        uint32_t arp_ip_source;
        //! Hardware destination address.
        uint8_t  arp_eth_dest[6];
        //! IP destination address.
        uint32_t arp_ip_dest;
} __attribute__ ((packed)) arp_t;

#endif
