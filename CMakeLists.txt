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

option(BUILD_SHARED_LIBS "Build libraries as shared" ON)
option(ENABLE_ASAN "Enable AddressSanitizer instrumentation" OFF)

# check if multi-config generator
get_property(PDXCP_IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

# only Debug and Release builds available
set(PDXCP_BUILD_CONFIGS Debug Release)
# use Debug as the default build
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()
# check that user didn't specify something unrecognized
if(CMAKE_BUILD_TYPE IN_LIST PDXCP_BUILD_CONFIGS)
else()
    message(
        FATAL_ERROR
        "Error: CMAKE_BUILD_TYPE must be one of ${PDXCP_BUILD_CONFIGS}"
    )
endif()
# if single-config generator, print config
if(NOT PDXCP_IS_MULTI_CONFIG)
    message(STATUS "Config: ${CMAKE_BUILD_TYPE}")
endif()

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
