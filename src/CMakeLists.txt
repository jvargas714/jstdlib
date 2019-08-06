
set(LIB_FILES
        jstd_util.h
        jstd_util.cpp
        udp_server.h
        net_types.h
        tcp_server.h
        CsvFileReader.h
        DataProcessor.h
        csvTypes.h
        CsvRow.h
        CsvDataProcessor.h
        fd_sets.h
        fd_sets.cpp
        IPAddress.cpp
        IPAddress.h
        TcpSocket.h
        TcpSocket.cpp
        net_exceptions.h
        )

if (MULTITHREADED)
    find_package(Threads REQUIRED)
    message("building target with multithreaded support")
endif()

add_library(jstdlib SHARED ${LIB_FILES})
set_target_properties(jstdlib PROPERTIES LINKER_LANGUAGE CXX)
