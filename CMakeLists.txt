cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(nachtlicht)

target_sources(app PRIVATE src/main.cpp src/connectivity/connectivity.cpp src/rgb_led/rgb_led.cpp)
target_include_directories(app PRIVATE src)
