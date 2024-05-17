cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(iot-assignment)

target_add_binary_data(${CMAKE_PROJECT_NAME}.elf "res/mqtt_cert.pem" TEXT)
