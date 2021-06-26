#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <termios.h>
#include <time.h>
#include <stdlib.h>

#include "serial.h"
#include "satgps.h"


/* globals */



int main(int argc, char **argv) {
    int sfd;
    char buffer[256];
    char sentence[256];
    char error[256];
    int nbytes;
    int gps_message_type;

    /* Open GPS device for reading */
    gps_open();

    /* infinite read loop */
    while (1) {

        /* read sentence from GPS device */
        gps_read(buffer);
        strcpy(sentence, buffer);

        /* make sure the data is valid before parsing */
        if (checksum_valid(buffer)) {

            /* parse sentence */
            if (parse_sentence(buffer) < 0) {
                gps_get_error(error);
                printf("GPS Error: %s\n", error);
            };

            //print_gsv();
            //print_gsv();
            //print_gll();

            print_rmc();
            //print_vtg();
            //print_gga();
            //print_gsa();

            /* usually filled when there's an error */
            print_txt();
        } else {
            printf("Checksum invalid for string: %s\n", buffer);
        }
    }
}

