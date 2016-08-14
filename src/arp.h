/**
 * Declare the external functions used by arp
 */
#ifndef __ARP_H__
#define __ARP_H__

#include <inttypes.h>

extern int send_arp(char *interface, uint32_t start_ip, uint32_t end_ip, uint32_t delay);
#endif 
