cmake_minimum_required(VERSION 3.16)
project(gpslib)

file(GLOB SERIAL_SRC
        "src/serial.*"
        )

file(GLOB GPSLIB_SRC
        "src/gpslib.*"
        )

file(GLOB GPSTEST_SRC
        "src/gpslib_tester.c"
        )

add_library(gps SHARED ${SERIAL_SRC} ${GPSLIB_SRC})

add_executable(gpslib_tester ${SERIAL_SRC} ${GPSLIB_SRC} ${GPSTEST_SRC})

target_compile_options(gpslib_tester PRIVATE -g)
