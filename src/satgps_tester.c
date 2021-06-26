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

/* GSV Data */
extern gsv_data_t GsvDataGlonass;          /* GLONASS GSV data */
extern gsv_data_t GsvDataGps;              /* GPS GSV data */
extern gll_data_t GllDataGn;               /* Combined GPS and GLONASS GLL data */
extern rmc_data_t RmcDataGn;               /* Combined GPS and GLONASS RMC data */
extern vtg_data_t VtgDataGn;               /* Combined GPS and GLONASS VTG data */
extern gga_data_t GgaDataGn;               /* Combined GPS and GLONASS GGA data */
extern gsa_data_t GsaDataGn;               /* Combined GPS and GLONASS GSA data */
extern txt_data_t TxtDataGn;               /* Combined GPS and GLONASS GSA data */



int main(int argc, char **argv) {
    int sfd;
    char buffer[255];
    char sentence[255];
    int nbytes;
    int gps_message_type;

    if (serial_open() < 0) {
        printf("Error: cannot open GPS port: %s\n", PORTNAME);
        return -1;
    }

    while (1) {
        nbytes = serial_readln(buffer);
        strcpy(sentence,buffer);
        if (checksum_valid(buffer)) {
            gps_message_type = parse_sentence(buffer);
            switch (gps_message_type) {
                case GLGSV_MESSAGE:
                    //print_gsv(GsvDataGlonass);
                    break;
                case GPGSV_MESSAGE:
                    //print_gsv(GsvDataGps);
                    break;
                case GNGLL_MESSAGE:
                    //print_gll(GllDataGn);
                    break;
                case GNRMC_MESSAGE:
                    //printf("%s\n",sentence);
                    print_rmc(RmcDataGn);
                    break;
                case GNVTG_MESSAGE :
                    //printf("%s\n",sentence);
                    //print_vtg(VtgDataGn);
                    break;
                case GNGGA_MESSAGE:
                    //print_gga(GgaDataGn);
                    break;
                case GNGSA_MESSAGE:
                    //print_gsa(GsaDataGn);
                    break;
                case GNTXT_MESSAGE:
                    /* Usually an error, so should always print */
                    print_txt(TxtDataGn);
                    break;
                default:
                    printf("Unknown sentence: %s\n", sentence);
            }
        } else {
            printf("Checksum invalid for string: %s\n", buffer);
        }
    }
}

