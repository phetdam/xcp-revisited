cmake_minimum_required(VERSION 3.21)

# project version values and full version
set(PDXCP_VERSION_MAJOR 0)
set(PDXCP_VERSION_MINOR 1)
set(PDXCP_VERSION_PATCH 0)
set(
    PDXCP_VERSION
    ${PDXCP_VERSION_MAJOR}.${PDXCP_VERSION_MINOR}.${PDXCP_VERSION_PATCH}
)

project(
    xcp-revisited
    VERSION ${PDXCP_VERSION}
    DESCRIPTION "Expert C Programming exercises"
    HOMEPAGE_URL https://github.com/phetdam/xcp-revisited
    LANGUAGES C
)

option(ENABLE_ASAN "Enable AddressSanitizer instrumentation" OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# eliminate per-configuration subdirs from being appended to build artifact
# output dirs for multi-config generators, e.g. Visual Studio, while outputting
# all the artifacts into the top-level build directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}$<$<CONFIG:Debug>:>)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}$<$<CONFIG:Debug>:>)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}$<$<CONFIG:Debug>:>)

# update CMake module path for our CMake scripts
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# add compile options
include(ConfigCompile)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(src)
