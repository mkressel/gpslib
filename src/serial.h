#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>

#ifndef PORTNAME
#define PORTNAME    "/dev/serial0"
#endif

// returns -1 if error, otherwise 0
int serial_open();
int serial_close();
int serial_readln(char *);

