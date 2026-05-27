cmake_minimum_required(VERSION 3.20)

#Only used for development

include(FetchContent)

project(vl53l5cx_esphome LANGUAGES C CXX)

find_package (Python3 COMPONENTS Interpreter)
execute_process (COMMAND "${Python3_EXECUTABLE}" -m venv "${CMAKE_CURRENT_SOURCE_DIR}/.venv")

# update the environment with VIRTUAL_ENV variable (mimic the activate script)
set (ENV{VIRTUAL_ENV} "${CMAKE_CURRENT_SOURCE_DIR}/.venv")
# change the context of the search
set (Python3_FIND_VIRTUALENV FIRST)
# unset Python3_EXECUTABLE because it is also an input variable (see documentation, Artifacts Specification section)
unset (Python3_EXECUTABLE)
# Launch a new search
find_package (Python3 COMPONENTS Interpreter)
message(STATUS "Python found: ${Python3_EXECUTABLE}")
message(STATUS "Python_SITELIB: ${Python3_SITELIB}")

execute_process (COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/.venv/bin/pip" install pip --upgrade)
execute_process (COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/.venv/bin/pip" install esphome --upgrade)

FetchContent_Declare(
        "VL53L5CX_ULD_API"
        GIT_REPOSITORY "https://github.com/SG-O/VL53L5CX_ULD_API.git"
        SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.libraries/VL53L5CX_ULD_API"
        GIT_TAG "origin/master"
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
)
FetchContent_MakeAvailable(VL53L5CX_ULD_API)

FetchContent_Declare(
        "VL53L8CX_ULD_API"
        GIT_REPOSITORY "https://github.com/SG-O/VL53L8CX_ULD_API.git"
        SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.libraries/VL53L8CX_ULD_API"
        GIT_TAG "origin/master"
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
)
FetchContent_MakeAvailable(VL53L8CX_ULD_API)

FetchContent_Declare(
        "VL53L_ULD_API_platform"
        GIT_REPOSITORY "https://github.com/SG-O/VL53L_ULD_API_platform.git"
        SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.libraries/VL53L_ULD_API_platform"
        GIT_TAG "origin/master"
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
)
FetchContent_MakeAvailable(VL53L_ULD_API_platform)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_definitions(-DUSE_ESP32)

# Path to the esphome package (provides esphome/core/... and esphome/components/... headers)
set(ESPHOME_ROOT "${Python3_SITELIB}")

# Automatically pick up all C/C++ sources under components/vl53l_mz
file(GLOB_RECURSE VL53L_MZ_SOURCES
    components/vl53l_mz/*.cpp
    components/vl53l_mz/*.c
)

add_library(vl53l_mz STATIC ${VL53L_MZ_SOURCES})

target_include_directories(vl53l_mz
    PUBLIC
        # Resolves: #include "esphome/core/..." and #include "esphome/components/..."
        ${ESPHOME_ROOT}
        # Resolves VL53L5CX_ULD_API headers
        ${CMAKE_CURRENT_SOURCE_DIR}/.libraries/VL53L5CX_ULD_API/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/.libraries/VL53L8CX_ULD_API/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/.libraries/VL53L_ULD_API_platform/inc
        # Resolves local headers in the root component dir
        ${CMAKE_CURRENT_SOURCE_DIR}/components/vl53l_mz
        # Resolves local headers in subdirectories (e.g. #include "../vl53l_mz_child.h")
        ${CMAKE_CURRENT_SOURCE_DIR}/components/vl53l_mz/sensor
        ${CMAKE_CURRENT_SOURCE_DIR}/components/vl53l_mz/text_sensor
)
