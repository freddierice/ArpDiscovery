#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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
