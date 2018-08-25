#include "logger.h"
#include "udp_server.h"
#include <iostream>

std::string g_ipaddr;
uint16_t g_port;

void handle_args(int argc, char** argv) {
    if (argc != 3) {
        std::cout << "applying default server ipaddr: " << LOCALHOSTIP << " port: " << g_port << std::endl;
        g_port = DEFAULT_UDP_SERVER_PORT;
        g_ipaddr = LOCALHOSTIP;
    } else {
        g_ipaddr = std::string(argv[1]);
        g_port = static_cast<uint16_t >(std::strtol(argv[2], nullptr, 10));
    }
}

int main(int argc, char** argv) {
    jstd::UdpServer<jstd::NetItem> srvr(g_ipaddr, g_port);
    srvr.run();
    srvr.join_threads();  // blocking
    return 0;
}
