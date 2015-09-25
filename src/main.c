#include <stdio.h>  // for io
#include <stdlib.h> // in case
#include <error.h>
#include <string.h> // for strlen
#include <net/if.h> // for IFNAMSIZ
#include <pthread.h> // for pthreads

#include <sys/types.h> // stat and geteuid
#include <sys/stat.h>
#include <unistd.h>

#include <getopt.h> //getopt

#include "arp.h"
#include "util.h"

#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define MAX_PATHLEN 4096
void printusage(int argc, char **argv);

char interface[IFNAMSIZ+1];

// set the default path
char iplink_path[MAX_PATHLEN+1] = "/bin/ip";

int main(int argc, char **argv){

    char c;

    struct option opts[] = {
        {"ip", 1, NULL, 'i'},
        {"range", 1, NULL, 'r'},
    };

/**
 * Get the options for the program
 */
    while(((c = getopt_long(argc, argv, "i:r:R:", opts, NULL)) != (char)-1) ){
        switch(c){
            case 'i': 
                strncpy(iplink_path, optarg, MAX_PATHLEN);
                break;
            default:
                fprintf(stderr, "Unkown option %c\n", c);
        }
    }

    // set the interface to the last value
    if(optind > 0 && optind < argc ){
        strncpy(interface, argv[optind],IFNAMSIZ);
    }else{
        printusage(argc, argv);
        return 1;
    }

/**
 * Check to see if running as root
 */
    if(geteuid()) { // non root user
        fprintf(stderr, "You must run this program as root! This is needed to open" \
                " raw sockets and capture raw packets.\n");
        return 1;
    }

/**
 * Done checking the inputs! Start the program logic
 */
    printf("Starting %s\n",argv[0]);
    
    // set the ip utility
    if(sysarp_set(iplink_path,interface,0) < 0){
        fprintf(stderr, "Error pausing the system ARP replies.\n");
        goto cleanup;
    }

    if( send_arp(interface) < 0 ){
        fprintf(stderr, "Error sending ARPs on the interface.\n");
        goto cleanup;
    }

    printf("Completed succesfully\n");
    sleep(10);

cleanup:
    if(sysarp_set(iplink_path,interface,1) < 0){
        fprintf(stderr, "Could not resume the system ARP replies\n");
    }
    return 0;
}

void printusage(int argc, char **argv){
    // check inputs
    if(argc != 2){
        fprintf(stderr, "ArpDiscovery %d.%d\n" \
        "Usage: %s [--ip, -i] <interface name>\n" \
        "OPTIONS:\n" \
        "  --range/-r\t\t\t give bounds on the subnet\n" \
        "  --ip/-i\t\t\t use a non-default location of ip\n" \
        "EXAMPLES:\n" \
        "  %s wlan0\n" \
        "  %s --ip /bin/ip wlan0\n", VERSION_MAJOR, VERSION_MINOR, argv[0], argv[0],
        argv[0]);
    }
}
