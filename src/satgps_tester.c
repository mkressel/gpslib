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
    char buffer[255];
    char sentence[255];
    int nbytes;
    int gps_message_type;

    /* Open GPS device for reading */
    gps_open();

    /* infinite read loop */
    while (1) {

        /* read sentence from GPS device */
        gps_read(buffer);
        strcpy(sentence,buffer);

        /* make sure the data is valid before parsing */
        if (checksum_valid(buffer)) {

            /* parse sentence */
            gps_message_type = parse_sentence(buffer);

            /* choose action based on GPS message type */
            switch (gps_message_type) {
                case GLGSV_MESSAGE:
                    //print_gsv();
                    break;
                case GPGSV_MESSAGE:
                    //print_gsv();
                    break;
                case GNGLL_MESSAGE:
                    //print_gll();
                    break;
                case GNRMC_MESSAGE:
                    print_rmc();
                    break;
                case GNVTG_MESSAGE :
                    //print_vtg();
                    break;
                case GNGGA_MESSAGE:
                    //print_gga();
                    break;
                case GNGSA_MESSAGE:
                    //print_gsa();
                    break;
                case GNTXT_MESSAGE:
                    /* Usually an error message, so print out */
                    print_txt();
                    break;
                default:
                    printf("Unknown sentence: %s\n", sentence);
            }
        } else {
            printf("Checksum invalid for string: %s\n", buffer);
        }
    }
}

