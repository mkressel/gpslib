#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <signal.h>

#include "serial.h"
#include "satgps.h"


void shutdown() {
    printf("Ctrl-C Caught, shutting down...\n");
    gps_close();
    exit(0);
}


int main(int argc, char **argv) {
    int sfd;
    char buffer[256];
    char sentence[256];
    char error[256];
    int nbytes;
    int gps_message_type;

    signal(SIGINT, shutdown);

    /* Open GPS device for reading */
    gps_open();

    /* set filters to parse wanted sentences */
    gps_set_filters(GNRMC_MESSAGE);

    /* infinite read loop */
    while (1) {

        /* read sentence from GPS device */
        gps_read(buffer);
        strcpy(sentence, buffer);

        /* make sure the data is valid before parsing */
        if (!checksum_valid(buffer)) {
            printf("Checksum invalid for sentence: %s\n", sentence);
            continue;
        }

        if(!prefix_valid(buffer)) {
            printf("Unknown GPS prefix for sentence: %s\n", sentence);
            continue;
        }

        /* parse sentence */
        if (parse_sentence(buffer) < 0) {
            gps_get_error(error);
            printf("GPS Error: %s\n", error);
            printf("Sentence: %s\n", sentence);
        };

        print_gsv(GPGSV_MESSAGE);
        //print_gsv();
        //print_gll();

        print_rmc();
        //print_vtg();
        //print_gga();
        //print_gsa();

        /* usually filled when there's an error */
        //print_txt();

    }
}



