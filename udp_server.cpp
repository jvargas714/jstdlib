#include <chrono>
#include <functional>   // std::hash
#include "logger.h"
#include "udp_server.h"
#include <arpa/inet.h>
#include <string>       // std::to_string
using namespace jstd;

#define USVR LOG_MODULE::UDPSERVER

// default connection settings
template<typename QItem>
UdpServer<QItem>::UdpServer() {
    LOG_TRACE(USVR);
    m_svr_conn.ipAddr = LOCALHOSTIP;
    m_svr_conn.sock_type = SOCK_DGRAM;
    m_svr_conn.sa.sin_port = DEFAULT_UDP_SERVER_PORT;
    if(!inet_aton(LOCALHOSTIP, &m_svr_conn.sa.sin_addr)) {
        LOG_ERROR(USVR, "invalid ip address supplied errno #", errno, " descr: ", sockErrToString(errno));
        exit(FATAL_ERR::IP_INET_FAIL);
    }
    m_svr_conn.sa.sin_family = AF_INET;
    m_svr_conn.sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_svr_conn.sockfd < 0) {
        LOG_ERROR(USVR, "error creating udp socket discriptor errno # ", errno, " descr: ", sockErrToString(errno));
        exit(FATAL_ERR::SOCK_FAIL);
    }
    int rc = bind(m_svr_conn.sockfd, (const struct sockaddr*)&m_svr_conn.sa, sizeof(m_svr_conn.sa));
    if (rc < 0) {
        LOG_ERROR(USVR, "binding socket to addr failed errno #", errno, " descr: ", sockErrToString(errno));
        exit(FATAL_ERR::SOCK_BIND_FAIL);
    }
    LOG_INFO(USVR, "udpserver on IP: ", m_svr_conn.ipAddr, " port: ", m_svr_conn.sa.sin_port);
}

template<typename QItem>
UdpServer<QItem>::UdpServer(const std::string &ip, const in_port_t& port) {
    LOG_TRACE(USVR);
    m_svr_conn.ipAddr = ip;
    m_svr_conn.sock_type = SOCK_DGRAM;
    m_svr_conn.sa.sin_port = port;
    if(!inet_aton(m_svr_conn.sa.sin_port, &m_svr_conn.sa.sin_addr)) {
        LOG_ERROR(USVR, "invalid ip address supplied errno #", errno, " descr: ", sockErrToString(errno));
        exit(FATAL_ERR::IP_INET_FAIL);
    }
    m_svr_conn.sa.sin_family = AF_INET;
    m_svr_conn.sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_svr_conn.sockfd < 0) {
        LOG_ERROR(USVR, "error creating udp socket discriptor errno # ", errno, " descr: ", sockErrToString(errno));
        exit(FATAL_ERR::SOCK_FAIL);
    }
    int rc = bind(m_svr_conn.sockfd, (const struct sockaddr*)&m_svr_conn.sa, sizeof(m_svr_conn.sa));
    if (rc < 0) {
        LOG_ERROR(USVR, "binding socket to addr failed errno #", errno, " descr: ", sockErrToString(errno));
        exit(FATAL_ERR::SOCK_BIND_FAIL);
    }
    LOG_INFO(USVR, "udpserver with IP: ", m_svr_conn.ipAddr, " port: ", m_svr_conn.sa.sin_port);
}

template<typename QItem>
size_t jstd::UdpServer<QItem>::hash_conn(const NetConnection& conn) const {
    std::hash<std::string> hasher;
    return hasher(conn.ipAddr + std::to_string(conn.sa.sin_port));
}

template<typename QItem>
bool jstd::UdpServer<QItem>::add_client(const std::string& ip, const uint16_t& port) {
    LOG_TRACE(USVR);
    NetConnection conn;
    conn.ipAddr = ip;
    conn.sock_type = SOCK_DGRAM;
    conn.sa.sin_port = port;
    conn.sa.sin_family = AF_INET;
    conn.sa.sin_len = sizeof(sockaddr_in);
    int rc = inet_aton(conn.ipAddr.c_str(), &conn.sa.sin_addr);
    if (rc < 0) {
        LOG_ERROR(USVR, "there was an error converting ip str:", ip, " to binary form");
        return false;
    }
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        LOG_ERROR(USVR,
                  "there was an error creating sockect discriptor, eerno: #",
                  errno,
                  " descr: ",
                  sockErrToString(errno));
        return false;
    }
    add_client(conn);
    return true;
}

template<typename QItem>
void UdpServer<QItem>::add_client(const NetConnection& conn) {
#ifdef MULTITHREADED_SRVR
    std::lock_guard<std::mutex> lck(m_cmtx);
#endif
    m_client_connections[hash_conn(conn)] = conn;
}

// process item off the msg queue
/*
 * struct NetConnection {
        std::string ipAddr;
        sockaddr_in sa;
        uint32_t sock_type;
        int sockfd;
 */
template<typename QItem>
bool UdpServer<QItem>::process_item(const QItem &item) {
    LOG_INFO(USVR, "processing item with hash_id: ", item.id);
    std::string tmp_msg = "hello thanks for the message, unfortunately this Server does nothing, IMPLEMENT ME!!";

}

template<typename QItem>
bool jstd::UdpServer<QItem>::process_item(const QItem &item, uint64_t hash_id) {
    auto connection = m_client_connections.find(hash_id);
    if (connection == m_client_connections.end()) {
        LOG_WARNING(USVR, "failed to find active connection, ", "for hash_id: ", hash_id, "aborting operation");
        return false;
    }
    item.conn = connection->second;
    process_item(item);
}

template<typename QItem>
auto jstd::UdpServer<QItem>::lookup_client(const uint64_t &hash_id) {
#ifdef MULTITHREADED_SRVR

#endif
}




// ------------------------------------------MULTITHREAD SUPPORTED CODE-------------------------------------------------
#ifdef MULTITHREADED_SRVR
template<typename QItem>
void UdpServer<QItem>::msg_recving() {
    ;
}

template<typename QItem>
void jstd::UdpServer<QItem>::msg_processing() {
    ;
}

template<typename QItem>
bool jstd::UdpServer<QItem>::run() {
    g_recv_thread = std::thread(&UdpServer::msg_recving, this);
    q_proc_thread = std::thread(&UdpServer::msg_processing, this);
}

#endif    // THREADED REGION OF SOURCE
