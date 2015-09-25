#ifndef __NETUTIL_H__
#define __NETUTIL_H__
#include <inttypes.h>

extern int netutil_interface_index(const char *interface, int sockfd, int *ret);
extern int netutil_interface_hwaddr(const char *interface, int sockfd, void *ret);
extern int netutil_interface_ipaddr(const char *interface, int sockfd, void *ret);
extern int netutil_interface_subnet(const char *interface, int sockfd, 
        uint32_t ip, uint32_t *start, uint32_t *end);

#endif
