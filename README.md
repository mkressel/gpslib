A GPS Library to Parse NMEA sentences. Eventually, this will evolve into an app that can be compiled into NASA's Core Flight System (cFS, https://cfs.gsfc.nasa.gov/) using the OpenSatKit toolchain (https://github.com/OpenSatKit/OpenSatKit).

Built for the Beitian BN-220 GPS module on a Raspberry Pi 4B, but could be ported to any GPS module that outputs NMEA sentences to a serial device.

This will eventually be a static library that can be linked to from your project. It is a work in progress. 

Right now, it compiles into an executable, "satgps_tester" and a library "libsatgps.a" which parses the GPS output and loads it into variables which can be accessed accordingly.

### To Compile
		cd build/
		cmake ..
		make

### To Test
In the build/ folder run:

		./gpslib_tester
		
The tester will print out all the data contained in the global variables (there are a lot.) The idea is to have all the data ready to use, and an application can query any of the GPS data at ant time. 


### Data Handling

Data is loaded into a global variable of type **gps_data_t**. As NMEA sentences are parsed, the **gps_data_t** struct is filled with new data. The data remains until the variable is overwritten by a new parsed NMEA sentence.

**Care should be taken to make sure the data is valid and current before using it in any location-sensitive project.** 

Most GPS data sentences have time fields and/or is-valid flags, which can be used to validate data. GPS (I'm pretty sure) is not accurate enough to point a satellite on its own, but combined with data from other instruments, the GPS data in this library might be used to provide medium accuracy local coordinates with speed and (geocentric) vectors.

The **gpslib_tester** executable shows how to interact with the library. 

*Please note: This is a work in progress so the code is changing often. Things will likely break*

It's my intent to eventually use this GPS library to:

1. Build a cFS (NASA Core Flight System) application that sends GPS telemetry from the onboard device to the ground.
2. Interface with an SPA (Sun Pointing Algorithm) to accurately determine the direction of the sun based on time and (GPS) location. 
		
-MK
