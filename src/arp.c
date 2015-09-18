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

#include <pcap.h>

/**
 * arphdr does not contain the structure of the hardware and ip addresses in
 * the arp request. This is the extension copied from net/if_arp.h.
 */
struct arpext {
    unsigned char ar_sha;           // Sender hardware address
    unsigned char ar_sip[ETH_ALEN]; // Sender IP address
    unsigned char ar_tha;           // Target hardware address
    unsigned char ar_tip[ETH_ALEN]; // Target IP address
};

#define BUFSIZE (sizeof(struct ether_header) + \
                 sizeof(struct arphdr) + \
                 sizeof(struct arpext))  

/**
 * Sends arp messages to obtain mac addresses
 */
int send_arp(char *interface){

    char buf[BUFSIZE];
    int sockfd;               // socket file descriptor
    struct ifreq sockif;      // socket interface
    struct ether_header *eh;  // ethernet packet header
    struct arphdr *ah;        // arp header
    struct arpext *ae;        // arp extension
    struct sockaddr_ll bc_addr;  // broadcast address

    // setup the buffer structure
    memset(buf, 0, BUFSIZE);
    eh = (struct ether_header *)buf;
    ah = (struct arphdr *)(buf + sizeof(eh));
    ae = (struct arpext *)(buf + sizeof(eh) + sizeof(ah));
    
    // setup the broadcast address 
    memset(&bc_addr, 0, sizeof(struct sockaddr_ll));
    bc_addr.sll_halen = ETH_ALEN; // set the ethernet addres length

    // setup ethernet headers
    eh->ether_type = ETHERTYPE_ARP;

    // get a RAW socket
    if ((sockfd = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW)) == -1){
        perror("Could not open raw socket");
        return -1;
    }
    
    // get the interface index
    memset(&sockif, 0, sizeof(struct ifreq));
    strncpy(sockif.ifr_name, interface, IFNAMSIZ-1);
    if (ioctl(sockfd, SIOCGIFINDEX, &sockif)) {
        perror("Could find the interface");
        return -1;
    }
    bc_addr.sll_ifindex = sockif.ifr_ifindex;   // set interface index

    // get the hardware address of 'interface'
    memset(&sockif.ifr_ifru, 0, sizeof(sockif.ifr_ifrn)); // clear out results section
    if( ioctl(sockfd, SIOCGIFADDR, &sockif) < 0){
        perror("Could not retrieve the hardware address from sockfd");
        return 1;
    }
    memcpy((void *)eh->ether_shost,(void *)&sockif.ifr_ifru, sizeof(struct sockaddr));
    memset((void *)eh->ether_dhost,0, sizeof(struct sockaddr));

    // send the packet
    if( sendto(sockfd, buf, BUFSIZE, 0, (struct sockaddr *)&bc_addr, sizeof(struct sockaddr_ll)) < 0 ){
        perror("Could not send the arp packet");
        return -1;
    }
    
    return 0;
}
