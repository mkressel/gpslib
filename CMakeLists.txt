cmake_minimum_required(VERSION 3.16)
project(satgps)

file(GLOB SERIAL_SRC
        "src/serial.*"
        )

file(GLOB SATGPS_SRC
        "src/satgps.*"
        )

file(GLOB SATTEST_SRC
        "src/satgps_tester.c"
        )

add_library(satgps STATIC ${SERIAL_SRC} ${SATGPS_SRC})

add_executable(satgps_tester ${SERIAL_SRC} ${SATGPS_SRC} ${SATTEST_SRC})

target_compile_options(satgps_tester PRIVATE -g)
