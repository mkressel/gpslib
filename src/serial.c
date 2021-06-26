#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <string.h>


#include "serial.h"

int serial_fd;

/* opens serial port and returns fd if success, -1 if error */
int serial_open() {

    struct termios tty;

    serial_fd = open(PORTNAME, O_RDONLY | O_NOCTTY);
    if (serial_fd < 0) {
        printf("Error opening %s: %s\n", PORTNAME, strerror(errno));
        return -1;
    }

    // get attr from serial
    if (tcgetattr(serial_fd, &tty) < 0) {
        printf("Error from tcgetattr: %s\n", strerror(errno));
        return -1;
    }

    // don't set output speed, causes errors with BN-220 GPS module
    //cfsetospeed(&tty, (speed_t) B9600);
    cfsetispeed(&tty, (speed_t) B9600);

    tty.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;         /* 8-bit characters */
    tty.c_cflag &= ~PARENB;     /* no parity bit */
    tty.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
    tty.c_cflag &= ~CRTSCTS;    /* no hardware flowcontrol */

    /* setup for non-canonical mode */
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    //tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    tty.c_lflag &= ~(ICANON | ISIG | IEXTEN);
    tty.c_oflag &= ~OPOST;

    /* fetch bytes as they become available */
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 1;

    if (tcsetattr(serial_fd, TCSANOW, &tty) != 0) {
        printf("Error from tcsetattr: %s\n", strerror(errno));
        return -1;
    }

    // all looks good
    return 0;
}

/* reads until newline, puts line into buffer */
int serial_readln(char * buffer) {
    char c;
    char *b = buffer;
    int rx_length = -1;

    while(1) {
        rx_length = read(serial_fd, (void*)(&c), 1);

        if (rx_length <= 0) {
            //wait for messages
            sleep(1);
        } else {
            if (c == '\n') {
                *b++ = '\0';
                break;
            }
            *b++ = c;
        }
    }
    return strlen(buffer);
}

int serial_close() {
    return close(serial_fd);
}

