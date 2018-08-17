cmake_minimum_required(VERSION 3.12)
project(jstdLib)

set(CMAKE_CXX_STANDARD 14)
set(LIB_FILES
        udp_server.cpp
        udp_server.h
        net_types.h)


add_library(jstdLib SHARED ${LIB_FILES})
add_executable(testUdpServer testUdpServer.cpp)
add_executable(testUdpClient testUdpClient.cpp)

