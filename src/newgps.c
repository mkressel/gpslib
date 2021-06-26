#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <termios.h>
#include <time.h>
#include <stdlib.h>

#include "serial.h"
#include "newgps.h"


/* globals */

/* GSV Data */
gsv_data_t GsvDataGlonass;          /* GLONASS GSV data */
gsv_data_t GsvDataGps;              /* GPS GSV data */
gll_data_t GllDataGn;               /* Combined GPS and GLONASS GLL data */
rmc_data_t RmcDataGn;               /* Combined GPS and GLONASS RMC data */
vtg_data_t VtgDataGn;               /* Combined GPS and GLONASS VTG data */
gga_data_t GgaDataGn;               /* Combined GPS and GLONASS GGA data */
gsa_data_t GsaDataGn;               /* Combined GPS and GLONASS GSA data */
txt_data_t TxtDataGn;               /* Combined GPS and GLONASS TXT data */


int main(int argc, char **argv) {
    int sfd;
    char buffer[255];
    int nbytes;
    int i;

    if (serial_open() < 0) {
        printf("Error: cannot open GPS port: %s\n", PORTNAME);
        return -1;
    }

    while (1) {
        nbytes = serial_readln(buffer);
        if (checksum_valid(buffer)) {
            parse_sentence(buffer);
        } else {
            printf("Checksum invalid for string: %s\n", buffer);
        }
    }
}

/* parse NMEA sentences */

int parse_sentence(char *buffer) {

    /* GLONASS GSV - $GLGSV */
    if (strncmp(buffer, NMEA_PREFIX_GLGSV, 5) == 0) {
        printf("GLGSV\n");
        parse_gsv(buffer, &GsvDataGlonass, GLGSV_MESSAGE);
        //print_gsv(GsvDataGlonass);
        return 0;
    }

    /* GPS GSV - $GPGSV */
    if (strncmp(buffer, NMEA_PREFIX_GPGSV, 5) == 0) {
        printf("GPGSV\n");
        parse_gsv(buffer, &GsvDataGps, GPGSV_MESSAGE);
        //print_gsv(GsvDataGps);
        return 0;
    }

    /* Combined GPS and GLONASS - $GNGLL */
    if (strncmp(buffer, NMEA_PREFIX_GNGLL, 5) == 0) {
        printf("GNGLL\n");
        parse_gll(buffer, &GllDataGn, GNGLL_MESSAGE);
        print_gll(GllDataGn);
        return 0;
    }

    /* Combined GPS and GLONASS - $GNRMC */
    if (strncmp(buffer, NMEA_PREFIX_GNRMC, 5) == 0) {
        printf("GNRMC\n");
        parse_rmc(buffer, &RmcDataGn, GNRMC_MESSAGE);
        print_rmc(RmcDataGn);
        return 0;
    }

    /* Combined GPS and GLONASS - $GNVTG */
    if (strncmp(buffer, NMEA_PREFIX_GNVTG, 5) == 0) {
        printf("GNVTG\n");
        parse_vtg(buffer, &VtgDataGn, GNVTG_MESSAGE);
        print_vtg(VtgDataGn);
        return 0;
    }

    /* Combined GPS and GLONASS - $GNGGA */
    if (strncmp(buffer, NMEA_PREFIX_GNGGA, 5) == 0) {
        printf("GNGGA\n");
        parse_gga(buffer, &GgaDataGn, GNGGA_MESSAGE);
        //print_gga(GgaDataGn);
        return 0;
    }

    /* Combined GPS and GLONASS - $GNGSA */
    if (strncmp(buffer, NMEA_PREFIX_GNGSA, 5) == 0) {
        printf("GNGSA\n");
        parse_gsa(buffer, &GsaDataGn, GNGSA_MESSAGE);
        print_gsa(GsaDataGn);
        return 0;
    }

    /* Combined GPS and GLONASS - $GNTXT */
    if (strncmp(buffer, NMEA_PREFIX_GNTXT, 5) == 0) {
        printf("GNTXT\n");
        parse_txt(buffer, &TxtDataGn, GNTXT_MESSAGE);
        print_txt(TxtDataGn);
        return 0;
    }

    printf("Unknown prefix: %s\n", buffer);
    return -1;

}

/*
    GSV Fields:
    0	Message ID $GPGSV
    1	Total number of messages of this type in this cycle
    2	Message number
    3	Total number of SVs visible
    4	SV PRN number
    5	Elevation, in degrees, 90° maximum
    6	Azimuth, degrees from True North, 000° through 359°
    7	SNR, 00 through 99 dB (null when not tracking)
    8–11	Information about second SV, same format as fields 4 through 7
    12–15	Information about third SV, same format as fields 4 through 7
    16–19	Information about fourth SV, same format as fields 4 through 7
    20	The checksum data, always begins with *
 */

int parse_gsv(char *buffer, gsv_data_t *GsvData, int gsv_type) {

    int num_fields = 0;
    int processed_fields = 0;
    int i;
    char *field[GPS_MAX_FIELDS];
    //printf("buffer: %s\n", buffer);

    num_fields = parse_fields(buffer, field, GPS_MAX_FIELDS);
    if (num_fields < 1) {
        printf("Bad GSV parse of sentence: %s\n", buffer);
        return -1;
    }

    /* number of sentences in this data packet */
    GsvData->total_messages = atoi(field[1]);
    /* which sentence we are reading */
    GsvData->message_number = atoi(field[2]);
    /* num sats in view */
    GsvData->satellites_in_view = atoi(field[3]);

    /* read up to four satellites per sentence */
    for (i = 0; i < 4; i++) {

        /* for proper array indexing of satellites */
        int skip = (GsvData->message_number - 1) * 4;

        /* make sure we don't read beyond the number of parsed fields */
        processed_fields += 4;

        if (processed_fields > num_fields) break;
        /* make sure we adjust PRN number based on GSV type */
        GsvData->gsv_sat[i + skip].prn_number = get_prn_number(atoi(field[4 + (i * 4)]), gsv_type);
        GsvData->gsv_sat[i + skip].elevation = atoi(field[5 + (i * 4)]);
        GsvData->gsv_sat[i + skip].azimuth = atoi(field[6 + (i * 4)]);
        GsvData->gsv_sat[i + skip].signal_to_noise = atoi(field[7 + (i * 4)]);

    }

    return 0;

}

/*
    GLL Fields:
    0	Message ID $GPGLL
    1	Latitude in dd mm,mmmm format (0-7 decimal places)
    2	Direction of latitude N: North S: South
    3	Longitude in ddd mm,mmmm format (0-7 decimal places)
    4	Direction of longitude E: East W: West
    5	UTC of position in hhmmss.ss format
    6	Fixed text "A" shows that data is valid
    7	The checksum data, always begins with *
 */

int parse_gll(char *buffer, gll_data_t *GllData, int gll_type) {

    int num_fields = 0;
    char hours[3];
    char minutes[3];
    char seconds[3];
    char milliseconds[16];
    char *eptr;
    char *field[GPS_MAX_FIELDS];
    //printf("buffer: %s\n", buffer);

    num_fields = parse_fields(buffer, field, GPS_MAX_FIELDS);
    if (num_fields < 1) {
        printf("Bad GLL parse of sentence: %s\n", buffer);
        return -1;
    }

    // if invalid, do not save data
    if (strncmp(field[6], "A", 1) == 0) {
        GllData->valid = 1;
    } else {
        GllData->valid = 0;
        return -1;
    }

    GllData->latitude = strtod(field[1], &eptr) / 100.00;
    /* South is negative */
    if (strncmp(field[2], "S", 1) == 0) {
        GllData->latitude *= -1;
    }

    GllData->longitude = strtod(field[3], &eptr) / 100.00;
    /* West is negative */
    if (strncmp(field[4], "W", 1) == 0) {
        GllData->longitude *= -1;
    }

    strcpy(GllData->utc_time_string, field[5]);

    strncpy(hours, field[5], 2);
    hours[2] = '\0'; // null terminate
    strncpy(minutes, field[5] + 2, 2);
    minutes[2] = '\0'; // null terminate
    strncpy(seconds, field[5] + 4, 2);
    seconds[2] = '\0'; // null terminate
    strcpy(milliseconds, field[5] + 7); // already null terminated

    GllData->utc_time.tv_sec = (atoi(hours) * 3600) + (atoi(minutes) * 60) + atoi(seconds);
    GllData->utc_time.tv_usec = atoi(milliseconds) * 1000;

    return 0;
}

/*
    RMC Fields:
    0	Message ID $GPRMC
    1	UTC of position fix
    2	Status A=active or V=void
    3	Latitude
    4	Direction of latitude N: North S: South
    5	Longitude
    6	Direction of longitude E: East W: West
    7	Speed over the ground in knots
    8	Track angle in degrees (True)
    9	Date
    10	Magnetic variation in degrees
    11	The checksum data, always begins with *
 */

int parse_rmc(char *buffer, rmc_data_t *RmcData, int rmc_type) {
    int num_fields = 0;

    char day[3];
    char month[3];
    char year[3];

    char hours[3];
    char minutes[3];
    char seconds[3];

    char *eptr;
    char *field[GPS_MAX_FIELDS];
    //printf("buffer: %s\n", buffer);

    num_fields = parse_fields(buffer, field, GPS_MAX_FIELDS);
    if (num_fields < 1) {
        printf("Bad RMC parse of sentence: %s\n", buffer);
        return -1;
    }

    // if invalid, do not save data
    if (strncmp(field[2], "A", 1) == 0) {
        RmcData->valid = 1;
    } else {
        RmcData->valid = 0;
        return -1;
    }

    strcpy(RmcData->utc_time_string, field[1]);

    // parse time
    strncpy(hours, field[1], 2);
    hours[2] = '\0'; // null terminate
    strncpy(minutes, field[1] + 2, 2);
    minutes[2] = '\0'; // null terminate
    strncpy(seconds, field[1] + 4, 2);
    seconds[2] = '\0'; // null terminate

    RmcData->latitude = strtod(field[3], &eptr) / 100.00;
    /* South is negative */
    if (strncmp(field[4], "S", 1) == 0) {
        RmcData->latitude *= -1;
    }

    RmcData->longitude = strtod(field[5], &eptr) / 100.00;
    /* West is negative */
    if (strncmp(field[6], "W", 1) == 0) {
        RmcData->longitude *= -1;
    }

    RmcData->speed = strtod(field[7], &eptr) * METERS_PER_SECOND_PER_KNOT;
    RmcData->track_angle = strtod(field[8], &eptr);

    strcpy(RmcData->utc_date_string, field[9]);



    // parse date
    strncpy(day, field[9], 2);
    day[2] = '\0'; // null terminate
    strncpy(month, field[9] + 2, 2);
    month[2] = '\0'; // null terminate
    strncpy(year, field[9] + 4, 2);
    year[2] = '\0'; // null terminate

    RmcData->utc_date.tm_mday = atoi(day);
    RmcData->utc_date.tm_mon = atoi(month);
    RmcData->utc_date.tm_year = atoi(year) + 2000;
    RmcData->utc_date.tm_hour = atoi(hours);
    RmcData->utc_date.tm_min = atoi(minutes);
    RmcData->utc_date.tm_sec = atoi(seconds);

    RmcData->magnetic_variation = strtod(field[10], &eptr);

    return 0;

}

/*
 *
    0	Message ID $GPVTG
    1	Track made good (degrees true)
    2	T: track made good is relative to true north
    3	Track made good (degrees magnetic)
    4	M: track made good is relative to magnetic north
    5	Speed, in knots
    6	N: speed is measured in knots
    7	Speed over ground in kilometers/hour (kph)
    8	K: speed over ground is measured in kph
    9	The checksum data, always begins with *
 */
int parse_vtg(char *buffer, vtg_data_t *VtgData, int vtg_type) {

    int num_fields;
    char *field[GPS_MAX_FIELDS];
    char *eptr;

    //printf("%s\n", buffer);

    num_fields = parse_fields(buffer, field, GPS_MAX_FIELDS);
    if (num_fields < 1) {
        printf("Bad VTG parse of sentence: %s\n", buffer);
        return -1;
    }

    if (strncmp(field[2], "T", 1) == 0) {
        VtgData->track_true = strtod(field[1], &eptr);
    }

    if (strncmp(field[4], "M", 1) == 0) {
        VtgData->track_true = strtod(field[3], &eptr);
    }

    // convert from knots
    if (strncmp(field[6], "N", 1) == 0) {
        VtgData->speed = strtod(field[5], &eptr) * METERS_PER_SECOND_PER_KNOT;
    }

    // convert from kph
    // this will overwrite above value from knots, but we assume kph is more accurate
    if (strncmp(field[8], "K", 1) == 0) {
        VtgData->speed = strtod(field[7], &eptr) * METERS_PER_SECOND_PER_KPH;
    }

}


/*
    GGA message
    0	Message ID $GPGGA
    1	UTC of position fix
    2	Latitude
    3	Direction of latitude: N: North S: South
    4	Longitude
    5	Direction of longitude: E: East W: West
    6	GPS Quality indicator: 0: Fix not valid, 1: GPS fix, 2: Differential GPS fix, OmniSTAR VBS, 4: Real-Time Kinematic, fixed integers, 5: Real-Time Kinematic, float integers, OmniSTAR XP/HP or Location RTK
    7	Number of SVs in use, range from 00 through to 24+
    8	HDOP
    9	Orthometric height (MSL reference)
    10	M: unit of measure for orthometric height is meters
    11	Geoid separation
    12	M: geoid separation measured in meters
    13	Age of differential GPS data record, Type 1 or Type 9. Null field when DGPS is not used.
    14	Reference station ID, range 0000-4095. A null field when any reference station ID is selected and no corrections are received1.
    15  The checksum data, always begins with *

 */

int parse_gga(char *buffer, gga_data_t *GgaData, int gga_type) {
    int num_fields;
    char *field[GPS_MAX_FIELDS];
    char *eptr;

    char hours[3];
    char minutes[3];
    char seconds[3];
    char milliseconds[16];

    //printf("%s\n", buffer);

    num_fields = parse_fields(buffer, field, GPS_MAX_FIELDS);
    if (num_fields < 1) {
        printf("Bad GGA parse of sentence: %s\n", buffer);
        return -1;
    }

    strcpy(GgaData->utc_time_string, field[1]);

    // parse time
    strncpy(hours, field[1], 2);
    hours[2] = '\0'; // null terminate
    strncpy(minutes, field[1] + 2, 2);
    minutes[2] = '\0'; // null terminate
    strncpy(seconds, field[1] + 4, 2);
    seconds[2] = '\0'; // null terminate
    strcpy(milliseconds, field[1] + 7); // already null terminated

    GgaData->utc_time.tv_sec = (atoi(hours) * 3600) + (atoi(minutes) * 60) + atoi(seconds);
    GgaData->utc_time.tv_usec = atoi(milliseconds) * 1000;

    GgaData->latitude = strtod(field[2], &eptr) / 100.0;
    if (strncmp(field[3], "S", 1) == 0) {
        GgaData->latitude *= -1;
    }

    GgaData->longitude = strtod(field[4], &eptr) / 100.0;
    if (strncmp(field[5], "W", 1) == 0) {
        GgaData->longitude *= -1;
    }

    GgaData->gps_quality = atoi(field[6]);
    GgaData->number_svs = atoi(field[7]);
    GgaData->HDOP = strtod(field[8], &eptr);

    if (strncmp(field[10], "M", 1) == 0) {
        GgaData->orthometric_height = strtod(field[9], &eptr);
    }

    if (strncmp(field[12], "M", 1) == 0) {
        GgaData->geoid_separation = strtod(field[11], &eptr);
    }

    GgaData->age_of_differential = strtod(field[13], &eptr);
    GgaData->reference_id = atoi(field[14]);

}

/*
    GSA sentence
    0	Message ID $GPGSA
    1	Mode 1, M = manual, A = automatic
    2	Mode 2, Fix type, 1 = not available, 2 = 2D, 3 = 3D
    3-14	PRN number, 01 through 32 for GPS, 33 through 64 for SBAS, 64+ for GLONASS
    15	PDOP: 0.5 through 99.9
    16	HDOP: 0.5 through 99.9
    17	VDOP: 0.5 through 99.9
    18	The checksum data, always begins with *
 */

int parse_gsa(char *buffer, gsa_data_t *GsaData, int gsa_type) {
    int num_fields, i;
    char *field[GPS_MAX_FIELDS];
    char *eptr;

    //printf("%s\n", buffer);

    num_fields = parse_fields(buffer, field, GPS_MAX_FIELDS);
    if (num_fields < 1) {
        printf("Bad GLL parse of sentence: %s\n", buffer);
        return -1;
    }


    if (strncmp(field[1], "M", 1) == 0) {
        GsaData->mode_1 = 0;
    } else if (strncmp(field[1], "A", 1) == 0) {
        GsaData->mode_1 = 1;
    } else {
        printf("Invalid GSA Mode 1: %s\n", field[1]);
        return -1;
    }

    GsaData->mode_2 = atoi(field[2]);
    for (i = 0; i < 12; i++) {
        GsaData->prn_number[i] = atoi(field[3 + i]);
    }
    GsaData->PDOP = strtod(field[4], &eptr);
    GsaData->HDOP = strtod(field[5], &eptr);
    GsaData->VDOP = strtod(field[6], &eptr);

    return 0;
}

/*
    TXT sentence
    0	Message ID $GNTXT
    1	Total number of sentences, 01 to 99
    2	Sentence number, 01 to 99
    3   Text identifier, 01 to 99
    4   Text message

 */

int parse_txt(char *buffer, txt_data_t *TxtData, int txt_type) {
    int num_fields, i;
    char *field[GPS_MAX_FIELDS];
    char *eptr;
    char *message;
    char temp_buffer[256];

    int sentence_number, text_id;

    //printf("%s\n", buffer);
    strcpy(temp_buffer,buffer);

    num_fields = parse_fields(buffer, field, GPS_MAX_FIELDS);
    if (num_fields < 1) {
        printf("Bad TXT parse of sentence: %s\n", buffer);
        return -1;
    }

    TxtData->num_sentences = atoi(field[1]);
    sentence_number = atoi(field[2]);
    text_id = atoi(field[3]);

    message = strstr(temp_buffer,field[4]);

    // copy sentence into correct index
    if(sentence_number > 0 && sentence_number < 100) {
        strcpy(TxtData->message[sentence_number],message);
        TxtData->text_id[sentence_number] = text_id;
    } else {
        printf("Bad TXT sentence number: %d\n",sentence_number);
        return -1;
    }

    return 0;
}

void print_txt(txt_data_t TxtData) {
    int i;
    // sentence numbers start at 1
    for(i=1;i<=TxtData.num_sentences;i++) {
        printf("TXT Message %d\n", i);
        printf("Text ID: %d\n",TxtData.text_id[i]);
        printf("Message: %s\n",TxtData.message[i]);
    }
}

void print_gsa(gsa_data_t GsaData) {

    int i;
    printf("Mode 1: %d\n", GsaData.mode_1);
    printf("Mode 2: %d\n", GsaData.mode_2);
    for (i = 0; i < 12; i++) {
        printf("PRN Number: %d\n", GsaData.prn_number[i]);
    }
    printf("PDOP: %.6f\n", GsaData.PDOP);
    printf("HDOP: %.6f\n", GsaData.HDOP);
    printf("VDOP: %.6f\n", GsaData.VDOP);

}


void print_gga(gga_data_t GgaData) {
    printf("UTC Time string: %s\n", GgaData.utc_time_string);
    printf("UTC Time Seconds: %d Milliseconds %d\n", GgaData.utc_time.tv_sec, GgaData.utc_time.tv_usec);
    printf("Latitude: %.6f\n", GgaData.latitude);
    printf("Longitude: %.6f\n", GgaData.longitude);
    printf("GPS quality: %d\n", GgaData.gps_quality);
    printf("Number SVs: %d\n", GgaData.number_svs);
    printf("HDOP: %.6f\n", GgaData.HDOP);
    printf("Orthometric height: %.6f\n", GgaData.orthometric_height);
    printf("Geoid separation: %.6f\n", GgaData.geoid_separation);
    printf("Age of differential: %.6f\n", GgaData.age_of_differential);
    printf("Reference id: %d\n", GgaData.reference_id);

}


void print_vtg(vtg_data_t VtgData) {
    printf("Track, degrees true: %.6f\n", VtgData.track_true);
    printf("Track, degrees magnetic: %.6f\n", VtgData.track_magnetic);
    printf("Speed in m/s: %.6f\n", VtgData.speed);
}

void print_rmc(rmc_data_t RmcData) {

    printf("Time string: %s\n", RmcData.utc_time_string);
    printf("Latitude %.6f\n", RmcData.latitude);
    printf("Longitude %.6f\n", RmcData.longitude);
    printf("Speed in m/s: %.6f\n", RmcData.speed);
    printf("Track angle: %.6f\n", RmcData.track_angle);
    printf("UTC Date string: %s\n", RmcData.utc_date_string);
    printf("UTC Date: %4d-%02d-%02d %02d:%02d:%02d\n", RmcData.utc_date.tm_year, RmcData.utc_date.tm_mon,
           RmcData.utc_date.tm_mday, RmcData.utc_date.tm_hour, RmcData.utc_date.tm_min, RmcData.utc_date.tm_sec);
    printf("Magnetic variation: %.6f\n", RmcData.magnetic_variation);

}

/*
 * Note –
    $GPGSV indicates GPS and SBAS satellites. If the PRN is greater than 32, this indicates an SBAS PRN, 87 should be added to the GSV PRN number to determine the SBAS PRN number.
    $GLGSV indicates GLONASS satellites. 64 should be subtracted from the GSV PRN number to determine the GLONASS PRN number.
    $GBGSV indicates BeiDou satellites. 100 should be subtracted from the GSV PRN number to determine the BeiDou PRN number.
    $GAGSVindicates Galileo satellites.
    $GQGSVindicates QZSS satellites.
    Source: https://www.trimble.com/OEM_ReceiverHelp/v5.11/en/NMEA-0183messages_GSV.html
 */

int get_prn_number(int prn, int gsv_type) {

    switch (gsv_type) {
        case GLGSV_MESSAGE: /* $GLGSV */
            return (prn - 64);
            break;
        case GPGSV_MESSAGE: /* $GPGSV */
            if (prn > 32) {
                return (prn + 87);
            }
            return prn;
            break;
        default:
            printf("Unknown GSV Type: %02x\n", gsv_type);
            return -1;
    }
}

void print_gsv(gsv_data_t GsvData) {

    int i;
    printf("===  Current GSV data ===\n");
    printf("Total messages: %d\n", GsvData.total_messages);
    printf("Message number: %d\n", GsvData.message_number);
    printf("Satellites in view: %d\n", GsvData.satellites_in_view);

    for (i = 0; i < GsvData.satellites_in_view; i++) {
        printf("\nSatellite number: %d\n", (i + 1));
        printf("PRN number: %d\n", GsvData.gsv_sat[i].prn_number);
        printf("Elevation: %d\n", GsvData.gsv_sat[i].elevation);
        printf("Azimuth: %d\n", GsvData.gsv_sat[i].azimuth);
        printf("Signal to noise dBm: %d\n", GsvData.gsv_sat[i].signal_to_noise);
    }
}

void print_gll(gll_data_t GllData) {
    printf("===  Current GLL data ===\n");
    printf("Latitude: %.6f\n", GllData.latitude);
    printf("Longitude: %.6f\n", GllData.longitude);
    printf("UTC Time Raw String: %s\n", GllData.utc_time_string);
    printf("UTC Time Seconds: %d Milliseconds %d\n", GllData.utc_time.tv_sec, GllData.utc_time.tv_usec);
}

int checksum_valid(char *string) {
    char *checksum_str;
    int checksum;
    unsigned char calculated_checksum = 0;

    // Checksum is postcede by *
    checksum_str = strchr(string, '*');
    if (checksum_str != NULL) {
        // Remove checksum from string
        *checksum_str = '\0';
        // Calculate checksum, starting after $ (i = 1)
        for (int i = 1; i < strlen(string); i++) {
            calculated_checksum = calculated_checksum ^ string[i];
        }
        checksum = hex2int((char *) checksum_str + 1);
        //printf("Checksum Str [%s], Checksum %02X, Calculated Checksum %02X\r\n",(char *)checksum_str+1, checksum, calculated_checksum);
        if (checksum == calculated_checksum) {
            //printf("Checksum OK");
            return 1;
        }
    } else {
        //printf("Error: Checksum missing or NULL NMEA message\r\n");
        return 0;
    }
    return 0;
}

int hex2int(char *c) {
    int value;

    value = hexchar2int(c[0]);
    value = value << 4;
    value += hexchar2int(c[1]);

    return value;
}

int hexchar2int(char c) {
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    return -1;
}

int parse_fields(char *string, char **fields, int max_fields) {
    int i = 0;
    fields[i++] = string;

    while ((i < max_fields) && NULL != (string = strchr(string, ','))) {
        *string = '\0';
        fields[i++] = ++string;
    }

    return --i;
}

/*int parse_fields(char *buffer, char **fields, int max_fields) {
    int i = 0;
    char *pt;
    char sentence[256];

    strcpy(sentence, buffer);

    pt = strtok(sentence, ",");
    while (pt != NULL) {
        fields[i] = pt;
        printf("fields[%d]: %s\n",i, fields[i]);
        i++;
        pt = strtok(NULL, ",");
    }
    return i;
}*/
