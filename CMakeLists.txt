cmake_minimum_required(VERSION 3.12)
project(jstdLib)

set(MULTITHREADED ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_VERBOSE_MAKEFILE ON)

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
    find_package(Threads REQUIRED)
    message("building target with multithreaded support")
    add_definitions(-DMULTITHREADED_SRVR)
endif()

add_library(jstdlib SHARED ${LIB_FILES})
set_target_properties(jstdlib PROPERTIES LINKER_LANGUAGE CXX)

# test executables
add_executable(testUdpServer testing/testUdpServer.cpp)
add_executable(testUdpClient testing/testUdpClient.cpp)

add_executable(testUdpServerApp testing/testUdpServerApp.cpp)
target_include_directories(testUdpServerApp PUBLIC ./)
link_directories(./)
if (MULTITHREADED)
    target_link_libraries(testUdpServerApp jstdlib Threads::Threads)
else()
    target_link_libraries(testUdpServerApp jstdlib)
endif()

add_executable(scrap testing/scrap.cpp)
target_include_directories(scrap PUBLIC ./)
