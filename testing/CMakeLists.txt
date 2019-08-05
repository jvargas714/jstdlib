include_directories(../src)
# -------------------- test executables -------------------------------------
find_package(Threads REQUIRED)

#add_executable(testUdpServer testUdpServer.cpp)
add_executable(testUdpClient testUdpClient.cpp)
add_executable(testUdpServerApp testUdpServerApp.cpp)
#target_include_directories(testUdpServerApp PUBLIC ./)
link_directories(./)

if (MULTITHREADED)
    target_link_libraries(testUdpServerApp jstdlib Threads::Threads)
else()
    target_link_libraries(testUdpServerApp jstdlib)
endif()

add_executable(testTcpServerApp testTcpServerApp.cpp)
add_executable(testTcpClient testTcpClient.cpp)
#target_include_directories(testTcpServerApp PUBLIC ./)

if (MULTITHREADED)
    target_link_libraries(testTcpServerApp jstdlib Threads::Threads)
else()
    target_link_libraries(testTcpServerApp jstdlib)
endif()

# other test files
add_executable(testTcpServer testTcpServer.cpp)

add_executable(scrap scrap.cpp)
#target_include_directories(scrap PUBLIC ./)
