# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2 CACHE STRING "Board type")
set(PICO_BOARD pico2)
set(PICO_PLATFORM rp2350)

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(lora_tx_sensores C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/lib/RadioLib-master/" "${CMAKE_CURRENT_BINARY_DIR}/RadioLib")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/lib/onewire_library/")

# Add executable. Default name is the project name, version 0.1

add_executable(lora_tx_sensores lora_tx_sensores.cpp)

pico_set_program_name(lora_tx_sensores "lora_tx_sensores")
pico_set_program_version(lora_tx_sensores "0.1")

# Generate PIO header
pico_generate_pio_header(lora_tx_sensores ${CMAKE_CURRENT_LIST_DIR}/blink.pio)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(lora_tx_sensores 0)
pico_enable_stdio_usb(lora_tx_sensores 1)

# Add the standard library to the build
target_link_libraries(lora_tx_sensores PRIVATE
        pico_stdlib
        hardware_spi
        hardware_pio
        hardware_pwm
        pico_multicore
        RadioLib
        onewire_library)

pico_add_extra_outputs(lora_tx_sensores)

