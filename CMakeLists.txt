cmake_minimum_required(VERSION 3.12)
project(jstdLib)

# todo :: eventually create a release configuration

set(BUILD_MODE DEBUG)
set(MULTITHREADED ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_VERBOSE_MAKEFILE OFF)

# set appropriate compiler flags
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    message("COMPILER SELECTED: CLANG")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wextra -pedantic")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    message("COMPILER SELECTED: APPLE CLANG")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wextra -pedantic")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    message("COMPILER SELECTED: GCC")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wextra -pedantic")
else()
    message("${CMAKE_CXX_COMPILER_ID}")
    error("UNKNOWN COMPILER, exiting")
endif()

# system OS identification
if (UNIX)
    if(APPLE)
        message("building targets for MAC OS X")
        add_definitions(-DOSX)
    else()
        message("building targets for Linux System")
        add_definitions(-DLINUX_OS)
    endif()

elseif(WIN32)
    message("building targets for Windows")
    add_definitions(-DWIN)
else()
    message("building target for unknown system")
endif()

if (MULTITHREADED)
    add_definitions(-DMULTITHREADED_SRVR)
endif()

# output dirs
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_subdirectory(./src)
add_subdirectory(./testing)
add_subdirectory(./third_party)