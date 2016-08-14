#include <stdio.h>  // for io
#include <stdlib.h> // in case
#include <error.h>
#include <string.h> // for strlen
#include <net/if.h> // for IFNAMSIZ
#include <pthread.h> // for pthreads

#include <sys/socket.h> // for inet_addr
#include <netinet/in.h>
#include <arpa/inet.h>

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

    int err = 0;
    char c;
	uint32_t start_ip = 0, end_ip = 0;
	int delay = 100;

    struct option opts[] = {
        {"start", 1, NULL, 's'},
        {"end", 1, NULL, 'e'},
        {"delay", 1, NULL, 'd'},
    };

/**
 * Get the options for the program
 */
    while(((c = getopt_long(argc, argv, "s:S:e:E:d:D", opts, NULL)) != (char)-1) ){
        switch(c){
			case 's':
				start_ip = ntohl((uint32_t)inet_addr(optarg));
				break;
			case 'e':
				end_ip = ntohl((uint32_t)inet_addr(optarg));
				break;
			case 'd':
				delay = atoi(optarg);
				break;
            default:
                fprintf(stderr, "Unkown option %c\n", c);
				printusage(argc, argv);
				exit(1);
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
    // set the ip utility
    if(sysarp_set(iplink_path,interface,0) < 0){
        fprintf(stderr, "Error pausing the system ARP replies.\n");
        err = 1; goto cleanup;
    }

	//debugging: printf("doing send_arp on %s with %u, %u, and %u\n", interface, start_ip, end_ip, delay);

	printf("starting... ");
    if( send_arp(interface, start_ip, end_ip, delay) < 0 ){
        fprintf(stderr, "Error sending ARPs on the interface.\n");
        err = 1; goto cleanup;
    }
	printf("done!\n");

cleanup:
    if(sysarp_set(iplink_path,interface,1) < 0){
        fprintf(stderr, "Could not resume the system ARP replies\n");
    }
    return err;
}

void printusage(int argc, char **argv){
    // check inputs
    if(argc != 2){
        fprintf(stderr, "arp-discovery %d.%d\n" \
        "Usage: %s [-s, -e, -d] <interface name>\n" \
        "OPTIONS:\n" \
        "  --start/-s\t\t\t give a start address to search\n" \
        "  --end/-e\t\t\t give an end address to search\n" \
        "  --delay/-d\t\t\t the delay between each arp send (default 100)\n" \
        "EXAMPLES:\n" \
        "  %s wlan0\n" \
        "  %s -s \"192.168.0.1\" wlan0\n", VERSION_MAJOR, VERSION_MINOR, argv[0], argv[0],
        argv[0]);
    }
}
