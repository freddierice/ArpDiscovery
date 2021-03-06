#include <stdio.h>      // standard io
#include <stdlib.h>     // standard lib operations
#include <string.h>     // string operations

#include <sys/types.h>      // socket types
#include <sys/socket.h>     // sockets
#include <sys/ioctl.h>      // special socket control

#include <netinet/in.h>      // IPPROTO_RAW
// #include <netinet/ip.h>      // ip proto? 
#include <net/if.h>          // interface definitions
#include <net/if_arp.h>      // arp header
#include <linux/if_packet.h> // correct according to man packet(7)
#include <net/ethernet.h>    // for ethernet header

#include <unistd.h>          // sleep in microseconds

#include <pcap.h>

#include "netutil.h"

/**
 * arphdr does not contain the structure of the hardware and ip addresses in
 * the arp request. This is the extension copied from net/if_arp.h.
 */
struct arpext {
    unsigned char ar_sha[ETH_ALEN]; // Sender hardware address
    unsigned char ar_sip[0x4];      // Sender IP address
    unsigned char ar_tha[ETH_ALEN]; // Target hardware address
    unsigned char ar_tip[0x4];      // Target IP address
};

#define BUFSIZE (sizeof(struct ether_header) + \
                 sizeof(struct arphdr) + \
                 sizeof(struct arpext))  

/**
 * Sends arp messages to obtain mac addresses
 */
int send_arp(char *interface, uint32_t start_ip, uint32_t end_ip, int delay){

    char buf[BUFSIZE];
    int sockfd;                 // socket file descriptor
    struct ether_header *eh;    // ethernet packet header
    struct arphdr *ah;          // arp header
    struct arpext *ae;          // arp extension
    struct sockaddr_ll bc_addr; // broadcast address
    uint32_t ip_iter;           // iterator for the ip addresses
	uint32_t ip_subnet_start; 	// start and ends to the ip subnet
	uint32_t ip_subnet_end; 	

    // setup the buffer structure
    memset(buf, 0, BUFSIZE);
    eh = (struct ether_header *)buf;
    ah = (struct arphdr *)((char *)buf + sizeof(struct ether_header));
    ae = (struct arpext *)(buf + sizeof(struct ether_header) + sizeof(struct arphdr));
    
    // setup the broadcast address 
    memset(&bc_addr, 0, sizeof(struct sockaddr_ll));
    bc_addr.sll_halen = ETH_ALEN; // set the ethernet addres length

    // setup ethernet headers
    eh->ether_type = htons(ETHERTYPE_ARP);

    // get a RAW socket
    if ((sockfd = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW)) == -1){
        perror("Could not open raw socket");
        return -1;
    }
    
    // get the interface index
    if(netutil_interface_index(interface, sockfd, &bc_addr.sll_ifindex) < 0)
        return -1;

    // get the interface hwaddr
    if(netutil_interface_hwaddr(interface, sockfd, eh->ether_shost) < 0 ||
            netutil_interface_hwaddr(interface, sockfd, &ae->ar_sha) < 0)
        return -1;
    
    // destination and target addresses are ff:ff:ff:ff:ff:ff
    memset((void *)eh->ether_dhost,0xff, ETH_ALEN); // ethernet broadcast is 0xff... 
    memset((void *)&ae->ar_tha,0xff, ETH_ALEN); // arp broadcast is 0xff...
    
    // get the ip address of 'interface'
    if( netutil_interface_ipaddr(interface, sockfd, (void *)&ae->ar_sip) < 0 )
        return -1;
    // ae->ar_tip[0] = 0x8a; ae->ar_tip[1] = 0x10; ae->ar_tip[2] = 0x1a; ae->ar_tip[3] = 0x1;

    // get the subnet mask
    if( netutil_interface_subnet(interface, sockfd, *((uint32_t *)ae->ar_sip), 
                &ip_subnet_start, &ip_subnet_end) < 0)
        return -1;

	// make sure the range is legit
	if( start_ip == 0 )
		start_ip = ip_subnet_start;
	if( end_ip == 0 )
		end_ip = ip_subnet_end;

	// make sure the delay is legit
	if( delay == 0 )
		delay = 100;

    // setup the arp header
    ah->ar_hrd = htons(ARPHRD_ETHER);
    ah->ar_pro = htons(ETHERTYPE_IP);
    ah->ar_hln = ETH_ALEN;
    ah->ar_pln = 0x4; // TODO: value found in wireshark transmission. would be nice to find the 
                      // actual constant. 
    ah->ar_op = htons(ARPOP_REQUEST);
    
    // setup the arp extension
    // TODO: setup more intelligent addresses
    
    // start ip loop
    for(ip_iter = start_ip; ip_iter < end_ip; ++ip_iter){
        // change target ip
        *(uint32_t *)&ae->ar_tip = htonl(ip_iter);

        // send the packet
        if( sendto(sockfd, buf, BUFSIZE, 0, (struct sockaddr *)&bc_addr, sizeof(struct sockaddr_ll)) < 0 ){
            perror("Could not send the arp packet");
            return -1;
        }
        usleep(delay);
    }

//  // simple debugging of packets
//  printf("PACKET---\n");
//  int i;
//  for(i = 0; i < BUFSIZE; ++i){
//      printf("%2x",(unsigned char)buf[i]);
//  }
//  printf("\n---PACKET\n");
    
    return 0;
}
