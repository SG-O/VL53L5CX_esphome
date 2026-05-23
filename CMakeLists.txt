cmake_minimum_required(VERSION 3.20)

project(vl53l5cx_esphome LANGUAGES C CXX)

find_package (Python3 COMPONENTS Interpreter)
execute_process (COMMAND "${Python3_EXECUTABLE}" -m venv "${CMAKE_CURRENT_SOURCE_DIR}/.venv")

# Here is the trick
## update the environment with VIRTUAL_ENV variable (mimic the activate script)
set (ENV{VIRTUAL_ENV} "${CMAKE_CURRENT_SOURCE_DIR}/.venv")
## change the context of the search
set (Python3_FIND_VIRTUALENV FIRST)
## unset Python3_EXECUTABLE because it is also an input variable (see documentation, Artifacts Specification section)
unset (Python3_EXECUTABLE)
## Launch a new search
find_package (Python3 COMPONENTS Interpreter)
message(STATUS "Python found: ${Python3_EXECUTABLE}")
message(STATUS "Python_SITELIB: ${Python3_SITELIB}")

execute_process (COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/.venv/bin/pip" install pip --upgrade)
execute_process (COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/.venv/bin/pip" install esphome --upgrade)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_definitions(-DUSE_ESP32)

# Path to the esphome package (provides esphome/core/... and esphome/components/... headers)
set(ESPHOME_ROOT "${Python3_SITELIB}")

# Automatically pick up all C/C++ sources under components/vl53l5cx
file(GLOB_RECURSE VL53L5CX_SOURCES
    components/vl53l5cx/*.cpp
    components/vl53l5cx/*.c
)

add_library(vl53l5cx STATIC ${VL53L5CX_SOURCES})

target_include_directories(vl53l5cx
    PUBLIC
        # Resolves: #include "esphome/core/..." and #include "esphome/components/..."
        ${ESPHOME_ROOT}
        # Resolves local headers in the root component dir
        ${CMAKE_CURRENT_SOURCE_DIR}/components/vl53l5cx
        # Resolves local headers in subdirectories (e.g. #include "../vl53l5cx_child.h")
        ${CMAKE_CURRENT_SOURCE_DIR}/components/vl53l5cx/sensor
        ${CMAKE_CURRENT_SOURCE_DIR}/components/vl53l5cx/text_sensor
        ${CMAKE_CURRENT_SOURCE_DIR}/components/vl53l5cx/uld
)
