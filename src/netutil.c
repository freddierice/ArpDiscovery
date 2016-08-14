/**
 * This is a set of functions that will allow a user to obtain interface
 * information quickly and easily
 */

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

#include "netutil.h"

// the socket interface used for this library.
struct ifreq sockif;

/**
 * Ready the interface for a function
 */  
void setup_interface_struct(const char *interface){
    memset(&sockif,0,sizeof(struct ifreq));
    strncpy(sockif.ifr_name, interface, IFNAMSIZ-1);
}

/**
 * Gets the interface's index
 */
int netutil_interface_index(const char *interface, int sockfd, int *ret){

    setup_interface_struct(interface);
    if (ioctl(sockfd, SIOCGIFINDEX, &sockif)) {
        perror("Could find the interface");
        return -1;
    }
    *ret = sockif.ifr_ifindex;
    return 0;
}

/**
 * Gets interface's hw address
 */
int netutil_interface_hwaddr(const char *interface, int sockfd, void *ret){

    setup_interface_struct(interface);
    if( ioctl(sockfd, SIOCGIFHWADDR, &sockif) < 0){
        perror("Could not retrieve the hardware address of the interface");
        return -1;
    }

    memcpy(ret, (void *)&sockif.ifr_ifru.ifru_hwaddr.sa_data, ETH_ALEN);

    return 0;
}

/**
 * Gets the interface's address. Since ARP is only a protocol on ipv4, we 
 * don't need to think about ipv6 address size.
 */
int netutil_interface_ipaddr(const char *interface, int sockfd, void *ret) {

    setup_interface_struct(interface);
    memset(&sockif.ifr_ifru, 0, sizeof(sockif.ifr_ifrn)); // clear out results section
    if( ioctl(sockfd, SIOCGIFADDR, &sockif) < 0){
        perror("Could not retrieve the ip address of the interface");
        return -1;
    }

    memcpy(ret, 
        (void *)&(((struct sockaddr_in *)&sockif.ifr_ifru.ifru_addr)->sin_addr), 
        0x4);

    return 0;
}

/**
 * Gets the interface's subnet
 */
int netutil_interface_subnet(const char *interface, int sockfd, uint32_t ip, uint32_t *start
        , uint32_t *end){
    
    setup_interface_struct(interface);
    if( ioctl(sockfd, SIOCGIFNETMASK, &sockif) < 0){
        perror("Could not retrieve the subnet from the interface");
        return 1;
    }
    memcpy((void *)start,
        (void *)&(((struct sockaddr_in *)&sockif.ifr_ifru.ifru_netmask)->sin_addr), 
        0x4);
    *end = *start;
    *start &= ip;
    *start = ntohl(*start) + 1;
    *end = ~(ntohl(*end)) - 1 + *start;

    return 0;
}
