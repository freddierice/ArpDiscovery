#include <stdio.h>  // for io
#include <stdlib.h> // in case
#include <error.h>
#include <string.h> // for strlen
#include <net/if.h> // for IFNAMSIZ

#include "arp.h"

int main(int argc, char **argv){

    char *interface;

    // check inputs
    if(argc != 2){
        fprintf(stderr, "Usage: %s <interface name>\n", argv[0]);
        return 1;
    }

    interface = argv[1];

    if(strlen(interface) >= IFNAMSIZ){
        fprintf(stderr, "Interface name is too long.\n");
        return 1;
    }


    // done checking inputs! logic is here
    printf("Starting %s\n",argv[0]);

    if( send_arp(interface) < 0 ){
        printf("Exited with an error\n");
        return 1;
    }

    printf("Completed succesfully\n");

    return 0;
}
