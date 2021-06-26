A GPS Library to Parse NMEA sentences. Eventually, this will evolve into an app that can be compiled into NASA's Core Flight System (cFS, https://cfs.gsfc.nasa.gov/) using the OpenSatKit toolchain (https://github.com/OpenSatKit/OpenSatKit).

Built for the Beitian BN-220 GPS module on a Raspberry Pi 4B, but could be ported to any GPS module that outputs NMEA sentences to a serial device.

This will eventually be a static library that can be linked to from your project. It is a work in progress. 

Right now, it compiles into an executable, "newgps" which parses the GPS output and loads it into variables which can be accessed accordingly.

### To Compile
		cd build
		make

### To Test
In the build/ folder run:
		gpslib_tester
