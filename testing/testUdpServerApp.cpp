#include "logger.h"
#include "udp_server.h"
#include <iostream>
#include "jstd_util.h"

std::string g_ipaddr;
uint16_t g_port;

void handle_args(int argc, char** argv) {
    if (argc != 3) {
        g_port = DEFAULT_UDP_SERVER_PORT;
        g_ipaddr = LOCALHOSTIP;
        std::cout << "applying default server ipaddr: " << LOCALHOSTIP << " port: " << g_port << std::endl;
    } else {
        g_ipaddr = std::string(argv[1]);
        g_port = static_cast<uint16_t>(std::strtol(argv[2], nullptr, 10));
    }
}

int main(int argc, char** argv) {
    handle_args(argc, argv);
    jstd::UdpServer<jstd::net::NetItem> srvr(g_ipaddr, g_port);
//    srvr.set_recv_timeout(10);
    srvr.set_nonblocking(false);
    srvr.run();
    srvr.join_threads();
    jstd::util::chrono::sleep_milli(5000);
    return 0;
}
