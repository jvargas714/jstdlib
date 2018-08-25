#include <chrono>
#include <functional>   // std::hash
#include "logger.h"
#include "udp_server.h"
#include <arpa/inet.h>
#include <string>       // std::to_string
#include <fcntl.h>      // fcntl()
using namespace jstd;

#define USVR LOG_MODULE::UDPSERVER
#define LOCK_GUARD_CLIENT_MAP std::lock_guard<std::mutex> lckm(m_cmtx);
#define LOCK_GUARD_ITEM_QUEUE std::lock_guard<std::mutex> lckq(m_qmtx);

// default connection settings
template<typename QItem>
UdpServer<QItem>::UdpServer()
: m_qproc_active(false), m_recv_active(false), m_is_bcast(false) {
    LOG_TRACE(USVR);
    m_svr_conn.ip_addr = LOCALHOSTIP;
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
#ifdef MULTITHREADED_SRVR
    m_is_nonblocking = false;
#endif
    LOG_INFO(USVR, "udpserver on IP: ", m_svr_conn.ip_addr, " port: ", m_svr_conn.sa.sin_port);
}

template<typename QItem>
UdpServer<QItem>::UdpServer(const std::string &ip, const in_port_t& port)
: m_qproc_active(false), m_recv_active(false), m_is_bcast(false) {
    LOG_TRACE(USVR);
    m_svr_conn.ip_addr = ip;
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
#ifdef MULTITHREADED_SRVR
    m_is_nonblocking = false;
#endif
    LOG_INFO(USVR, "udpserver with IP: ", m_svr_conn.ip_addr, " port: ", m_svr_conn.sa.sin_port);
}

template<typename QItem>
jstd::UdpServer<QItem>::~UdpServer() {
    LOG_TRACE(USVR);
    LOG_DEBUG(USVR, "\n", m_stats);
}

template<typename QItem>
uint64_t jstd::UdpServer<QItem>::hash_conn(const NetConnection& conn) const {
    return hash_conn(conn.ip_addr, conn.sa.sin_port);
}

template<typename QItem>
uint64_t jstd::UdpServer<QItem>::hash_conn(const std::string &ipaddr, const int& port) const {
    std::hash<std::string> hasher;
    return hasher(ipaddr + std::to_string(port));
}


template<typename QItem>
bool jstd::UdpServer<QItem>::add_client(const std::string& ip, const uint16_t& port) {
    LOG_TRACE(USVR);
    NetConnection conn;
    conn.ip_addr = ip;
    conn.sock_type = SOCK_DGRAM;
    conn.sa.sin_port = port;
    conn.sa.sin_family = AF_INET;
    int rc = inet_aton(conn.ip_addr.c_str(), &conn.sa.sin_addr);
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

// add client only if not currently in map
template<typename QItem>
void UdpServer<QItem>::add_client(const NetConnection& conn) {
#ifdef MULTITHREADED_SRVR
    LOCK_GUARD_CLIENT_MAP;
#endif
    uint64_t hash_id = hash_conn(conn);
    if (m_client_connections.find(hash_id) == m_client_connections.end())
        m_client_connections[hash_id] = conn;
    m_stats.clients_added_cnt++;
}

// process item off the msg queue
// assumes item has valid connection information
template<typename QItem>
bool UdpServer<QItem>::process_item(const QItem &item) {
    LOG_TRACE(USVR);
    LOG_INFO(USVR, "processing item recvd:\n", item);
    std::stringstream ss;
    for (const auto& el : item.buff) ss << el;
    LOG_INFO(USVR, "data recvd: ", ss.str());
    std::string tmp_msg = "hello thanks for the message, unfortunately this Server does nothing, IMPLEMENT ME!!\n";
    QItem resp;
    resp.buff = std::vector<uint8_t>( tmp_msg.begin(), tmp_msg.end() );
    resp.conn = item.conn;
    return send_item(resp);
}

template<typename QItem>
bool jstd::UdpServer<QItem>::process_item(QItem &&item) {
    LOG_TRACE(USVR);
    LOG_INFO(USVR, "processing rval ref item recvd:\n", item);
    std::stringstream ss;
    for (const auto& el : item.buff) ss << el;
    LOG_INFO(USVR, "data recvd: ", ss.str());
    std::string tmp_msg = "hello thanks for the message, unfortunately this Server does nothing, IMPLEMENT ME!!\n";
    QItem resp;
    resp.buff = std::vector<uint8_t>( tmp_msg.begin(), tmp_msg.end() );
    resp.conn = item.conn;
    return send_item(resp);
}

// process item, but perform a client lookup via hash_id
// typically this can be called from the recv thread
template<typename QItem>
bool jstd::UdpServer<QItem>::process_item(const QItem &item, uint64_t hash_id) {
    bool found = false;
    auto connection = lookup_client(hash_id, found);
    if (!found) {
        LOG_WARNING(USVR, "failed to find active connection, ", "for hash_id: ", hash_id, "aborting operation");
        return false;
    }
    item.conn = connection->second;
    return process_item(item);
}

template<typename QItem>
auto jstd::UdpServer<QItem>::lookup_client(const uint64_t &hash_id, bool& found) {
#ifdef MULTITHREADED_SRVR
    LOCK_GUARD_CLIENT_MAP;
#endif
    auto it = m_client_connections.find(hash_id);
    found = (it != m_client_connections.end());
    return it;
}

template<typename QItem>
bool jstd::UdpServer<QItem>::send_item(const QItem &item, uint64_t hash_id) {
    LOG_TRACE(USVR);
    if (m_is_bcast) {
        int num_clients = broadcast_data(item.serialize());
        LOG_DEBUG(USVR, num_clients, " clients have been broadcasted data to");
        return true;
    }
    bool found;
    auto client_it = lookup_client(hash_id, found);
    if (!found) {
        LOG_ERROR(USVR, "client with hash_id: ", hash_id, " not found, not sending message");
        return false;
    }
    item.conn = client_it->second;
    return send_item(item);
}

template<typename QItem>
bool jstd::UdpServer<QItem>::send_item(const QItem &item) {
    LOG_TRACE(USVR);
    std::vector<uint8_t> outBoundBuff = item.serialize();
    if (m_is_bcast) {
        int num_clients = broadcast_data(outBoundBuff);
        LOG_DEBUG(USVR, num_clients, " have been broadcasted data");
        return true;
    }
    ssize_t bytes_sent = sendto(item.conn.sockfd,
            outBoundBuff.data(),
            outBoundBuff.size(),
            0,
            (const struct sockaddr_in*)item.conn.sa,
            item.conn.addr_len);
    if (bytes_sent < 0) {
        LOG_ERROR(USVR,
                "failed to send data, errno# ",
                errno,
                " descr: ",
                sockErrToString(errno));
        return false;
    }
    LOG_INFO(USVR,
            "successfully sent out ",
            bytes_sent,
            " bytes of data to ",
            item.conn.ip_addr,
            ":",
            item.conn.sa.sin_port);
    return true;
}

template<typename QItem>
bool jstd::UdpServer<QItem>::set_nonblocking(bool is_nonblocking) {
    LOG_TRACE(USVR);
    if (is_nonblocking) {
        if (m_svr_conn.sockfd == INVALID_SOCKET) {
            LOG_ERROR(USVR, "svr listening socket is invalid");
            return false;
        }
        fcntl(m_svr_conn.sockfd, F_SETFL, O_NONBLOCK);
        if (fcntl(m_svr_conn.sockfd, F_GETFL) & O_NONBLOCK) {
            LOG_WARNING(USVR, "non blocking operation NOT set on listening socket");
            return false;
        } else {
            m_is_nonblocking = true;
            LOG_DEBUG(USVR, "non-blocking recv operation set on listening socket");
            return true;
        }
    } else {  // set back to blocking if nonblocking already set
        if (fcntl(m_svr_conn.sockfd, F_GETFL) & O_NONBLOCK) {
            fcntl(m_svr_conn.sockfd, F_SETFL,
                    fcntl(m_svr_conn.sockfd, F_GETFL) ^ O_NONBLOCK
                    );
            m_is_nonblocking = false;
        }
    }
    return true;
}

template<typename QItem>
bool jstd::UdpServer<QItem>::is_nonblocking() {
    return static_cast<bool>(fcntl(m_svr_conn.sockfd, F_GETFL) & O_NONBLOCK);
}

template<typename QItem>
bool jstd::UdpServer<QItem>::set_bcast_mode(bool is_set) {
    LOG_TRACE(USVR);
    if (is_set) {
        LOG_INFO(USVR, "setting server to broadcast mode, current client count: ",
                 m_client_connections.size());
    } else {
        LOG_INFO(USVR, "disabling broadcast mode on server");
    }
    m_is_bcast = is_set;
    return false;
}

template<typename QItem>
int jstd::UdpServer<QItem>::broadcast_data(const std::vector<uint8_t> &data) {
    LOG_TRACE(USVR);
    if (data.empty()) {
        LOG_WARNING(USVR, "data buffer empty, not bcasting data");
        return 0;
    }
    QItem item;
    int client_cnt = 0;
    item.buff = data;
    LOG_DEBUG(USVR, "broadcasting data to ", m_client_connections.size(), " clients");
#ifdef MULTITHREADED_SRVR
    LOCK_GUARD_CLIENT_MAP;
#endif
    for (const auto& client : m_client_connections) {
        item.conn = client;
        if (send_item(item)) {
            client_cnt++;
        }
        else {
            LOG_WARNING(USVR, "removing client: ", item.conn.ip_addr, ":", item.conn.sa.sin_port);
            _remove_client(item.conn.ip_addr, item.conn.sa.sin_port);
        }
    }
    LOG_DEBUG(USVR, "successfully sent data to ",
            client_cnt, "/", m_client_connections.size(), " clients");
    return client_cnt;
}

template<typename QItem>
void jstd::UdpServer<QItem>::clear_clients() {
#ifdef MULTITHREADED_SRVR
    LOCK_GUARD_CLIENT_MAP;
#endif
    m_client_connections.clear();
}

template<typename QItem>
bool jstd::UdpServer<QItem>::remove_client(const std::string& ipaddr, const int& port) {
    LOG_TRACE(USVR);
    LOG_DEBUG(USVR, "removing client ", ipaddr, ":", port);
#ifdef MULTITHREADED_SRVR
    LOCK_GUARD_CLIENT_MAP;
#endif
    return _remove_client();
}

template<typename QItem>
void jstd::UdpServer<QItem>::_build_qitem(QItem &item,
        const uint8_t *buff, const ssize_t &len, const sockaddr_in &addr) const {
    item.conn.ip_addr = std::string(inet_ntoa(addr.sin_addr));
    item.conn.sa = addr;
    item.buff = std::vector<uint8_t>(buff, buff + len);
}

// ------------------------------------------MULTITHREAD SUPPORT-------------------------------------------------
#ifdef MULTITHREADED_SRVR
template<typename QItem>
void UdpServer<QItem>::msg_recving() {
    LOG_TRACE(USVR);
    LOG_DEBUG(USVR, "message receiving thread started");
    uint8_t buff[MAX_BUFF_SIZE];
    std::memset(buff, 0, MAX_BUFF_SIZE);
    ssize_t num_bytes = 0;
    sockaddr_in from_addr = {0};
    socklen_t addr_len = sizeof(sockaddr_in);
    while(m_recv_active) {
        num_bytes = recvfrom(m_svr_conn.sockfd,
                buff,
                MAX_BUFF_SIZE,
                0,
                (struct sockaddr*)&from_addr,
                &addr_len);
        if (num_bytes == SOCKET_ERROR) {
            LOG_ERROR(USVR, "there was an error receiving udp packets errno#",
                    errno, " descr: ", sockErrToString(errno));
        } else {
            QItem item;
            _build_qitem(item, buff, num_bytes, from_addr);
            LOG_INFO(USVR, "recvd ", num_bytes, " bytes from ", item.ip_addr, ":", item.sa.sin_port);
            add_client(item.conn);
            push_qitem(item);
            m_stats.msg_recvd_cnt++;
            std::memset(buff, 0, MAX_BUFF_SIZE);
        }
    }
    LOG_DEBUG(USVR, "exiting message recv thread...");
}

template<typename QItem>
void jstd::UdpServer<QItem>::msg_processing() {
    LOG_TRACE(USVR);
    LOG_DEBUG(USVR, "message processing thread started");
    while(m_qproc_active) {
        std::this_thread::sleep_for(std::chrono::milliseconds(DEFAULT_SVR_THREAD_SLEEP));
        if (!m_msg_queue.empty()) {
            if ( process_item( std::move(m_msg_queue.front()) ) )
                m_stats.msg_processed_cnt++;
            m_qmtx.lock();
            m_msg_queue.pop();
            m_qmtx.unlock();
        }
    }
}

template<typename QItem>
bool jstd::UdpServer<QItem>::run() {
    LOG_DEBUG(USVR, "starting message receiving and item processing thread");
    m_qproc_active = true;
    m_recv_active = true;
    m_recv_thread = std::thread(&UdpServer::msg_recving, this);
    m_q_proc_thread = std::thread(&UdpServer::msg_processing, this);
}

template<typename QItem>
void jstd::UdpServer<QItem>::push_qitem(const QItem &item) {
    LOG_TRACE(USVR);
    LOCK_GUARD_ITEM_QUEUE;
    m_msg_queue.push(item);
}

template<typename QItem>
void jstd::UdpServer<QItem>::join_threads() {
    LOG_TRACE(USVR);
    LOG_DEBUG(USVR, "UDP server is now blocking, until app termination");
    m_recv_thread.join();
    m_q_proc_thread.join();
    LOG_DEBUG(USVR, "UDP server theads have exited");
}
#endif    // THREADED REGION OF SOURCE
