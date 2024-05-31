cmake_minimum_required(VERSION 3.17)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(CMakePrintHelpers)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH "" CACHE PATH "Path to Pico SDK")
cmake_print_variables(PICO_SDK_PATH)

set(WIFI_SSID "" CACHE STRING "WiFi SSID")
set(WIFI_PASSWORD "" CACHE STRING "WiFi Password")

set(MQTT_SERVER_HOST "" CACHE STRING "MQTT Server")
set(MQTT_SERVER_PORT "" CACHE STRING "MQTT Server Port")
set(MQTT_CLIENT_ID "" CACHE STRING "MQTT Client ID")
set(MQTT_USER "" CACHE STRING "MQTT Username")
set(MQTT_PASSWORD "" CACHE STRING "MQTT Password")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(thirdparty/pico-sdk/pico_sdk_init.cmake)

project(plantspy C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

if ("${WIFI_SSID}" STREQUAL "")
    message("Skipping build as WIFI_SSID is not defined")
elseif ("${WIFI_PASSWORD}" STREQUAL "")
    message("Skipping build as WIFI_PASSWORD is not defined")
else()
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in
        ${CMAKE_CURRENT_SOURCE_DIR}/src/config.h
        @ONLY
    )

    add_executable(plantspy
        src/main.c
        src/dht20.h
        src/dht20.c
        src/bus.h
        src/bus.c
        src/ads1115.h
        src/ads1115.c
        src/mqtt.h
        src/mqtt.c
        src/util.h
        src/util.c
        src/config.h
        src/lwipopts.h
        # src/mbedtls_config.h
    )

    pico_set_program_name(plantspy "PlantSpy")
    pico_set_program_version(plantspy "1.0")

    pico_enable_stdio_uart(plantspy 1)
    pico_enable_stdio_usb(plantspy 1)

    target_include_directories(plantspy PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/src # for our common lwipopts
    )

    target_link_libraries(plantspy
        pico_stdlib
        pico_cyw43_arch_lwip_poll
        pico_lwip_mbedtls
        pico_mbedtls
        pico_lwip_mqtt
        hardware_i2c
    )

    pico_add_extra_outputs(plantspy)
endif()
