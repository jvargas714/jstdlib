#include <chrono>
#include "logger.h"
#include "udp_server.h"
#include <arpa/inet.h>
using namespace jstd;

#define USVR LOG_MODULE::UDPSERVER

// default connection settings
template<typename QItem>
UdpServer<QItem>::UdpServer() {
    LOG_TRACE(USVR);
    svr_conn.ipAddr = LOCALHOSTIP;
    svr_conn.sock_type = SOCK_DGRAM;
    svr_conn.sa.sin_port = DEFAULT_UDP_SERVER_PORT;
    if(!inet_aton(LOCALHOSTIP, &svr_conn.sa.sin_addr)) {
        LOG_ERROR(USVR, "invalid ip address supplied errno #", errno, " descr: ", sockErrToString(errno));
        exit(FATAL_ERR::IP_INET_FAIL);
    }
    svr_conn.sa.sin_family = AF_INET;
    svr_conn.sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (svr_conn.sockfd < 0) {
        LOG_ERROR(USVR, "error creating udp socket discriptor errno # ", errno, " descr: ", sockErrToString(errno));
        exit(FATAL_ERR::SOCK_FAIL);
    }
    int rc = bind(svr_conn.sockfd, (const struct sockaddr*)&svr_conn.sa, sizeof(svr_conn.sa));
    if (rc < 0) {
        LOG_ERROR(USVR, "binding socket to addr failed errno #", errno, " descr: ", sockErrToString(errno));
        exit(FATAL_ERR::SOCK_BIND_FAIL);
    }
    LOG_INFO(USVR, "udpserver with IP: ", svr_conn.ipAddr, " port: ", svr_conn.sa.sin_port);
}
template<typename QItem>
UdpServer<QItem>::UdpServer(const std::string &ip, const in_port_t& port) {
    LOG_TRACE(USVR);
    svr_conn.ipAddr = ip;
    svr_conn.sock_type = SOCK_DGRAM;
    svr_conn.sa.sin_port = port;
    if(!inet_aton(svr_conn.sa.sin_port, &svr_conn.sa.sin_addr)) {
        LOG_ERROR(USVR, "invalid ip address supplied errno #", errno, " descr: ", sockErrToString(errno));
        exit(FATAL_ERR::IP_INET_FAIL);
    }
    svr_conn.sa.sin_family = AF_INET;
    svr_conn.sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (svr_conn.sockfd < 0) {
        LOG_ERROR(USVR, "error creating udp socket discriptor errno # ", errno, " descr: ", sockErrToString(errno));
        exit(FATAL_ERR::SOCK_FAIL);
    }
    int rc = bind(svr_conn.sockfd, (const struct sockaddr*)&svr_conn.sa, sizeof(svr_conn.sa));
    if (rc < 0) {
        LOG_ERROR(USVR, "binding socket to addr failed errno #", errno, " descr: ", sockErrToString(errno));
        exit(FATAL_ERR::SOCK_BIND_FAIL);
    }
    LOG_INFO(USVR, "udpserver with IP: ", svr_conn.ipAddr, " port: ", svr_conn.sa.sin_port);
}

template<typename QItem>
bool UdpServer<QItem>::processMsg(const QItem &item) {
    return false;
}

template<typename QItem>
void UdpServer<QItem>::msg_recving() {
    while() {

    }
}
