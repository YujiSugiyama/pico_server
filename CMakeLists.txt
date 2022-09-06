# Set minimum required version of CMake
cmake_minimum_required(VERSION 3.12)

set(PICO_CXX_ENABLE_EXCEPTIONS 1)

#include build functions from Pico SDK
include(pico_sdk_import.cmake)

# Set name of project (as PROJECT_NAME) and C/C++ Standards
project(project_name C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if(CMAKE_BUILD_TYPE MATCHES "Debug")
	add_compile_definitions(IS_DEBUG_MODE="1")
endif()

# Creates a pico-sdk subdirectory in our project for the libraries
pico_sdk_init()

# point out the CMake, where to find the executable source file
add_executable(pico_server
	src/main.cpp
	src/graphlib.c
	src/ili9341.c
	src/disp.c
	src/esp32.cpp
	src/server.cpp
	src/http.cpp
	src/core1.cpp
)

target_include_directories(pico_server PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_options(pico_server PUBLIC "-Wno-psabi")
target_compile_options(pico_server PUBLIC "-DPICO_STACK_SIZE=0x1800")
target_compile_options(pico_server PUBLIC "-DPICO_CORE1_STACK_SIZE=0x0800")
target_compile_options(pico_server PUBLIC "-DPICO_CORE1_STACK_BOTTOM=0x20040000")
#target_compile_options(pico_server PUBLIC "-DPICO_STACK_SIZE=0x4000")
#target_compile_options(pico_server PUBLIC "-DPICO_CORE1_STACK_SIZE=0x4000")
#target_compile_options(pico_server PUBLIC "-DPICO_CORE1_STACK_BOTTOM=0x2003A000")
target_compile_options(pico_server PUBLIC "-DPICO_HEAP_SIZE=0x2000")

pico_enable_stdio_usb(pico_server 0)
pico_enable_stdio_uart(pico_server 1)

# create map/bin/hex/uf2 files.
pico_add_extra_outputs(pico_server)

# Pull in our pico_stdlib which pulls in commonly used features (gpio, timer-delay etc)
target_link_libraries(pico_server
	pico_stdlib
	pico_multicore
	hardware_rtc
	hardware_spi
	hardware_pwm
)
