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

/* GPS Data */
gps_data_t GpsData;

/* opens port to GPS device for reading */

int gps_open() {
    if (serial_open() < 0) {
        return -1;
    } else {
        return 0;
    }
}

int gps_read(char *buffer) {
    return serial_readln(buffer);
}

/* copies the error message into error_string */
void gps_get_error(char *error_string) {
    strcpy(error_string,GpsData.error_message);
}

//void gps_do_error();

/* Parse NMEA sentences
 *
 * Returns message number or -1 if it does not understand the sentence
 *
 * */

int parse_sentence(char *buffer) {

    /* GLONASS GSV - $GLGSV */
    if (strncmp(buffer, NMEA_PREFIX_GLGSV, 5) == 0) {
        return parse_gsv(buffer, &GpsData.GsvDataGlonass, GLGSV_MESSAGE);
    }

    /* GPS GSV - $GPGSV */
    if (strncmp(buffer, NMEA_PREFIX_GPGSV, 5) == 0) {
        return parse_gsv(buffer, &GpsData.GsvDataGps, GPGSV_MESSAGE);
    }

    /* Combined GPS and GLONASS - $GNGLL */
    if (strncmp(buffer, NMEA_PREFIX_GNGLL, 5) == 0) {
        return parse_gll(buffer, &GpsData.GllDataGn, GNGLL_MESSAGE);
    }

    /* Combined GPS and GLONASS - $GNRMC */
    if (strncmp(buffer, NMEA_PREFIX_GNRMC, 5) == 0) {
        return parse_rmc(buffer, &GpsData.RmcDataGn, GNRMC_MESSAGE);
    }

    /* Combined GPS and GLONASS - $GNVTG */
    if (strncmp(buffer, NMEA_PREFIX_GNVTG, 5) == 0) {
        return parse_vtg(buffer, &GpsData.VtgDataGn, GNVTG_MESSAGE);
    }

    /* Combined GPS and GLONASS - $GNGGA */
    if (strncmp(buffer, NMEA_PREFIX_GNGGA, 5) == 0) {
        return parse_gga(buffer, &GpsData.GgaDataGn, GNGGA_MESSAGE);
    }

    /* Combined GPS and GLONASS - $GNGSA */
    if (strncmp(buffer, NMEA_PREFIX_GNGSA, 5) == 0) {
        return parse_gsa(buffer, &GpsData.GsaDataGn, GNGSA_MESSAGE);
    }

    /* Combined GPS and GLONASS - $GNTXT */
    if (strncmp(buffer, NMEA_PREFIX_GNTXT, 5) == 0) {
        return parse_txt(buffer, &GpsData.TxtDataGn, GNTXT_MESSAGE);
    }

    /* Unknown prefix */
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

    num_fields = parse_fields(buffer, field, GPS_MAX_FIELDS);
    if (num_fields < 1) {
        sprintf(GpsData.error_message,"Bad GSV parse of sentence: %s\n",buffer);
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
    char degrees[4];


    char milliseconds[16];
    char *eptr;
    char *field[GPS_MAX_FIELDS];

    num_fields = parse_fields(buffer, field, GPS_MAX_FIELDS);
    if (num_fields < 1) {
        sprintf(GpsData.error_message,"Bad GLL parse of sentence: %s\n", buffer);
        return -1;
    }

    // if invalid, do not save data
    if (strncmp(field[6], "A", 1) == 0) {
        GllData->valid = 1;
    } else {
        GllData->valid = 0;
        return -1;
    }

    strncpy(degrees, field[1], 2);
    degrees[2] = '\0'; // null terminate
    strcpy(minutes,field[1] + 2);

    GllData->latitude = strtod(degrees, &eptr) + (strtod(minutes, &eptr) / 60.00);
    /* South is negative */
    if (strncmp(field[2], "S", 1) == 0) {
        GllData->latitude *= -1;
    }

    strncpy(degrees, field[3], 3);
    degrees[3] = '\0'; // null terminate
    strcpy(minutes,field[3] + 3);

    GllData->longitude =  strtod(degrees, &eptr) + (strtod(minutes, &eptr) / 60.00);
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
    char degrees[4];

    char *eptr;
    char *field[GPS_MAX_FIELDS];

    num_fields = parse_fields(buffer, field, GPS_MAX_FIELDS);
    if (num_fields < 1) {
        sprintf(GpsData.error_message,"Bad RMC parse of sentence: %s\n", buffer);
        return -1;
    }

    // if invalid, do not save data
    if (strncmp(field[2], "A", 1) == 0) {
        RmcData->valid = 1;
    } else {
        RmcData->valid = 0;
        sprintf(GpsData.error_message,"RMC Data invalid: %s\n", buffer);
        return -1;
    }

    strcpy(RmcData->utc_time_string, field[1]);

    strncpy(degrees, field[3], 2);
    degrees[2] = '\0'; // null terminate
    strcpy(minutes,field[3] + 2);

    RmcData->latitude = strtod(degrees, &eptr) + (strtod(minutes, &eptr) / 60.00);

    /* South is negative */
    if (strncmp(field[4], "S", 1) == 0) {
        RmcData->latitude *= -1;
    }

    strncpy(degrees, field[5], 3);
    degrees[3] = '\0'; // null terminate
    strcpy(minutes,field[5] + 3);

    RmcData->longitude = strtod(degrees, &eptr) + (strtod(minutes, &eptr) / 60.00);

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

    // parse time
    strncpy(hours, field[1], 2);
    hours[2] = '\0'; // null terminate
    strncpy(minutes, field[1] + 2, 2);
    minutes[2] = '\0'; // null terminate
    strncpy(seconds, field[1] + 4, 2);
    seconds[2] = '\0'; // null terminate
    
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
    num_fields = parse_fields(buffer, field, GPS_MAX_FIELDS);
    if (num_fields < 1) {
        sprintf(GpsData.error_message,"Bad VTG parse of sentence: %s\n", buffer);
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
    char degrees[4];
    char milliseconds[16];

    num_fields = parse_fields(buffer, field, GPS_MAX_FIELDS);
    if (num_fields < 1) {
        sprintf(GpsData.error_message,"Bad GGA parse of sentence: %s\n", buffer);
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


    strncpy(degrees, field[2], 2);
    degrees[2] = '\0'; // null terminate
    strcpy(minutes,field[2] + 2);

    GgaData->latitude = strtod(degrees, &eptr) + (strtod(minutes, &eptr) / 60.00);

    if (strncmp(field[3], "S", 1) == 0) {
        GgaData->latitude *= -1;
    }

    strncpy(degrees, field[4], 3);
    degrees[3] = '\0'; // null terminate
    strcpy(minutes,field[4] + 3);

    GgaData->longitude = strtod(degrees, &eptr) + (strtod(minutes, &eptr) / 60.00);

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

    num_fields = parse_fields(buffer, field, GPS_MAX_FIELDS);
    if (num_fields < 1) {
        sprintf(GpsData.error_message,"Bad GLL parse of sentence: %s\n", buffer);
        return -1;
    }


    if (strncmp(field[1], "M", 1) == 0) {
        GsaData->mode_1 = 0;
    } else if (strncmp(field[1], "A", 1) == 0) {
        GsaData->mode_1 = 1;
    } else {
        sprintf(GpsData.error_message,"Invalid GSA Mode 1: %s\n", field[1]);
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

    strcpy(temp_buffer,buffer);

    num_fields = parse_fields(buffer, field, GPS_MAX_FIELDS);
    if (num_fields < 1) {
        sprintf(GpsData.error_message,"Bad TXT parse of sentence: %s\n", buffer);
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
        sprintf(GpsData.error_message,"Bad TXT sentence number: %d\n",sentence_number);
        return -1;
    }

    return 0;
}

void print_txt() {
    int i;
    printf("===  Current TXT data ===\n");
    // sentence numbers start at 1
    for(i=1;i<=GpsData.TxtDataGn.num_sentences;i++) {
        printf("TXT Message %d\n", i);
        printf("Text ID: %d\n",GpsData.TxtDataGn.text_id[i]);
        printf("Message: %s\n",GpsData.TxtDataGn.message[i]);
    }
}

void print_gsa() {

    int i;
    printf("===  Current GSA data ===\n");
    printf("Mode 1: %d\n", GpsData.GsaDataGn.mode_1);
    printf("Mode 2: %d\n", GpsData.GsaDataGn.mode_2);
    for (i = 0; i < 12; i++) {
        printf("PRN Number: %d\n", GpsData.GsaDataGn.prn_number[i]);
    }
    printf("PDOP: %.6f\n", GpsData.GsaDataGn.PDOP);
    printf("HDOP: %.6f\n", GpsData.GsaDataGn.HDOP);
    printf("VDOP: %.6f\n", GpsData.GsaDataGn.VDOP);

}


void print_gga() {
    printf("===  Current GGA data ===\n");
    printf("UTC Time string: %s\n", GpsData.GgaDataGn.utc_time_string);
    printf("UTC Time Seconds: %d Milliseconds %d\n", GpsData.GgaDataGn.utc_time.tv_sec, GpsData.GgaDataGn.utc_time.tv_usec);
    printf("Latitude: %.6f\n", GpsData.GgaDataGn.latitude);
    printf("Longitude: %.6f\n", GpsData.GgaDataGn.longitude);
    printf("GPS quality: %d\n", GpsData.GgaDataGn.gps_quality);
    printf("Number SVs: %d\n", GpsData.GgaDataGn.number_svs);
    printf("HDOP: %.6f\n", GpsData.GgaDataGn.HDOP);
    printf("Orthometric height: %.6f\n", GpsData.GgaDataGn.orthometric_height);
    printf("Geoid separation: %.6f\n", GpsData.GgaDataGn.geoid_separation);
    printf("Age of differential: %.6f\n", GpsData.GgaDataGn.age_of_differential);
    printf("Reference id: %d\n", GpsData.GgaDataGn.reference_id);

}


void print_vtg() {
    printf("===  Current VTG data ===\n");
    printf("Track, degrees true: %.6f\n", GpsData.VtgDataGn.track_true);
    printf("Track, degrees magnetic: %.6f\n", GpsData.VtgDataGn.track_magnetic);
    printf("Speed in m/s: %.6f\n", GpsData.VtgDataGn.speed);
}

void print_rmc() {

    printf("===  Current RMC data ===\n");
    printf("Time string: %s\n", GpsData.RmcDataGn.utc_time_string);
    printf("Latitude %.6f\n", GpsData.RmcDataGn.latitude);
    printf("Longitude %.6f\n", GpsData.RmcDataGn.longitude);
    printf("Speed in m/s: %.6f\n", GpsData.RmcDataGn.speed);
    printf("Track angle: %.6f\n", GpsData.RmcDataGn.track_angle);
    printf("UTC Date string: %s\n", GpsData.RmcDataGn.utc_date_string);
    printf("UTC Date: %4d-%02d-%02d %02d:%02d:%02d\n", GpsData.RmcDataGn.utc_date.tm_year, GpsData.RmcDataGn.utc_date.tm_mon,
           GpsData.RmcDataGn.utc_date.tm_mday, GpsData.RmcDataGn.utc_date.tm_hour, GpsData.RmcDataGn.utc_date.tm_min, GpsData.RmcDataGn.utc_date.tm_sec);
    printf("Magnetic variation: %.6f\n", GpsData.RmcDataGn.magnetic_variation);

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
            sprintf(GpsData.error_message,"Unknown GSV Type: %02x\n", gsv_type);
            return -1;
    }
}

void print_gsv() {

    int i;
    printf("===  Current GSV data ===\n");
    printf("Total messages: %d\n", GpsData.GsvDataGps.total_messages);
    printf("Message number: %d\n", GpsData.GsvDataGps.message_number);
    printf("Satellites in view: %d\n", GpsData.GsvDataGps.satellites_in_view);

    for (i = 0; i < GpsData.GsvDataGps.satellites_in_view; i++) {
        printf("\nSatellite number: %d\n", (i + 1));
        printf("PRN number: %d\n", GpsData.GsvDataGps.gsv_sat[i].prn_number);
        printf("Elevation: %d\n", GpsData.GsvDataGps.gsv_sat[i].elevation);
        printf("Azimuth: %d\n", GpsData.GsvDataGps.gsv_sat[i].azimuth);
        printf("Signal to noise dBm: %d\n", GpsData.GsvDataGps.gsv_sat[i].signal_to_noise);
    }
}

void print_gll() {
    printf("===  Current GLL data ===\n");
    printf("Latitude: %.6f\n", GpsData.GllDataGn.latitude);
    printf("Longitude: %.6f\n", GpsData.GllDataGn.longitude);
    printf("UTC Time Raw String: %s\n", GpsData.GllDataGn.utc_time_string);
    printf("UTC Time Seconds: %d Milliseconds %d\n", GpsData.GllDataGn.utc_time.tv_sec, GpsData.GllDataGn.utc_time.tv_usec);
}

/* 1 for valid, 0 for not */
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
        sprintf(GpsData.error_message,"Error: Checksum missing or NULL NMEA message\r\n");
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
