# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/pi/src/gps/newgps

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/pi/src/gps/newgps/build

# Include any dependencies generated for this target.
include CMakeFiles/satgps.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/satgps.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/satgps.dir/flags.make

CMakeFiles/satgps.dir/src/serial.c.o: CMakeFiles/satgps.dir/flags.make
CMakeFiles/satgps.dir/src/serial.c.o: ../src/serial.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/pi/src/gps/newgps/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/satgps.dir/src/serial.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/satgps.dir/src/serial.c.o   -c /home/pi/src/gps/newgps/src/serial.c

CMakeFiles/satgps.dir/src/serial.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/satgps.dir/src/serial.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/pi/src/gps/newgps/src/serial.c > CMakeFiles/satgps.dir/src/serial.c.i

CMakeFiles/satgps.dir/src/serial.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/satgps.dir/src/serial.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/pi/src/gps/newgps/src/serial.c -o CMakeFiles/satgps.dir/src/serial.c.s

CMakeFiles/satgps.dir/src/gpslib.c.o: CMakeFiles/satgps.dir/flags.make
CMakeFiles/satgps.dir/src/gpslib.c.o: ../src/gpslib.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/pi/src/gps/newgps/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/satgps.dir/src/gpslib.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/satgps.dir/src/gpslib.c.o   -c /home/pi/src/gps/newgps/src/gpslib.c

CMakeFiles/satgps.dir/src/gpslib.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/satgps.dir/src/gpslib.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/pi/src/gps/newgps/src/gpslib.c > CMakeFiles/satgps.dir/src/gpslib.c.i

CMakeFiles/satgps.dir/src/gpslib.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/satgps.dir/src/gpslib.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/pi/src/gps/newgps/src/gpslib.c -o CMakeFiles/satgps.dir/src/gpslib.c.s

# Object files for target satgps
satgps_OBJECTS = \
"CMakeFiles/satgps.dir/src/serial.c.o" \
"CMakeFiles/satgps.dir/src/gpslib.c.o"

# External object files for target satgps
satgps_EXTERNAL_OBJECTS =

libsatgps.so: CMakeFiles/satgps.dir/src/serial.c.o
libsatgps.so: CMakeFiles/satgps.dir/src/gpslib.c.o
libsatgps.so: CMakeFiles/satgps.dir/build.make
libsatgps.so: CMakeFiles/satgps.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/pi/src/gps/newgps/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking C shared library libsatgps.so"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/satgps.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/satgps.dir/build: libsatgps.so

.PHONY : CMakeFiles/satgps.dir/build

CMakeFiles/satgps.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/satgps.dir/cmake_clean.cmake
.PHONY : CMakeFiles/satgps.dir/clean

CMakeFiles/satgps.dir/depend:
	cd /home/pi/src/gps/newgps/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/pi/src/gps/newgps /home/pi/src/gps/newgps /home/pi/src/gps/newgps/build /home/pi/src/gps/newgps/build /home/pi/src/gps/newgps/build/CMakeFiles/satgps.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/satgps.dir/depend

