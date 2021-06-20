cmake_minimum_required(VERSION 3.16)
project(newgps)

file(GLOB GPS_SRC
        "src/*.h"
        "src/*.c"
        )

add_executable(newgps ${GPS_SRC} src/serial.c)
target_compile_options(newgps PRIVATE -g)
