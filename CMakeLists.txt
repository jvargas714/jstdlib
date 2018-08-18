cmake_minimum_required(VERSION 3.12)
project(jstdLib)

set(MULTITHREADED ON)
set(CMAKE_CXX_STANDARD 14)

# system OS identification
if (UNIX)
    if(APPLE)
        message("building targets for MAC OS X")
        add_definitions(-DOSX)
    endif()
    message("building targets for Linux System")
    add_definitions(-DLINUX)

elseif(WIN32)
    message("building targets for Windows")
    add_definitions(-DWIN)
else()
    message("building target for unknown system")
endif()

set(LIB_FILES
        udp_server.cpp
        udp_server.h
        net_types.h)

if (MULTITHREADED)
    add_definitions(-DMULTITHREADED_SRVR)
endif()

add_library(jstdLib SHARED ${LIB_FILES})
add_executable(testUdpServer testUdpServer.cpp)
add_executable(testUdpClient testUdpClient.cpp)

