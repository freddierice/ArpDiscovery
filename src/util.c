#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include <string.h>

#include "util.h"

const char *arg_link = "link";
const char *arg_set = "set";
const char *arg_dev = "dev";
const char *arg_arp = "arp";
const char *arg_on = "on";
const char *arg_off = "off";

// {ip, ip, link, set, dev, interface, arp, on/off, NULL}
char *argv[8]; 

/**
 * Runs command 'ip link set dev <interface> arp <on/off>'
 */
int sysarp_set(const char *iplink_path, const char *interface, int flag){
    int err = 0;

    // defaults 
    argv[1] = (char *)arg_link; argv[2] = (char *)arg_set;
    argv[3] = (char *)arg_dev; argv[5] = (char *)arg_arp; 
    
    // inputs
    argv[0] = (char *)iplink_path; 
    argv[4] = (char *)interface;
    argv[6] = flag ? (char *)arg_on : (char *)arg_off;
    argv[7] = NULL;
    if(!fork()){ // in the child
        err = execv((const char *)argv[0], argv);
    }

    return err;
}

int ip2uint32t(const char *ip, uint32_t *ret){

    return 0;
}

/**
 * Converts an ip range into a starting and ending ip address (uint32_t)
 */
int parse_ip(const char *iprange, uint32_t *start, uint32_t *end){
    
    int dash_flag = 0;
    char *iter;
    uint32_t ip1, ip2, ip3, ip4;

    // check for NULL ip range
    if(!iprange)
        return -1;

    // the length has to make sense
    if(strlen(iprange) > 31)
        return -1;

    // initialize the start and end ip addresses for convenience
    *start = *end = 0;

    // check for ok iprange format
    for(iter = (char *)iprange; *iter; ++iter){
        // only allow these characters
        if( (*iter < '0' || *iter > '9') && *iter != '.' && *iter != '-' )
            return -1;
        if(*iter == '-'){
            dash_flag++; 
            continue;
        }
    }

    // convert the ip addresses to uint32_t types. 
    if(dash_flag == 0){
        if( sscanf(iprange,"%u.%u.%u.%u", &ip1, &ip2, &ip3, &ip4) <= 0 )
            return -1;
        *start = (ip1 << 0x18) + (ip2 << 0x10) + (ip3 << 0x8) + ip4;

    }else if(dash_flag == 1){
        if( sscanf(iprange,"%u.%u.%u.%u", &ip1, &ip2, &ip3, &ip4) <= 0 )
            return -1;
        *start = (ip1 << 0x18) + (ip2 << 0x10) + (ip3 << 0x8) + ip4;
        
    }else{
        // this should not happen.
        return -1;
    }
    
    return 0;
}
