#ifndef JSTDLIB_NET_TYPES_H
#define JSTDLIB_NET_TYPES_H
#include <cstdint>
#include <cstring>
#include <string>
#include <memory>
#include <vector>
#include <ostream>
#include <sstream>
#include <netinet/in.h>
#include <sys/socket.h>


// increment udp ports by 5
#define DEFAULT_UDP_SERVER_PORT 5005

// increment tcp ports by 2
#define DEFAULT_TCP_SERVER_PORT 5002

// maximum buff size
#define MAX_BUFF_SIZE 2048

// localhost
#define LOCALHOSTIP "127.0.0.1"

// err codes
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

// server operating parameters
#define DEFAULT_SVR_THREAD_SLEEP 50 // in ms

// fatal error exit codes
enum FATAL_ERR {
    SOCK_FAIL = -10,
    IP_INET_FAIL,
    SOCK_BIND_FAIL
};

enum class SERVER_TYPE {
    SINGLE_THREADED,
    MULTITHREADED
};

namespace jstd {
#ifdef LINUX_OS
std::string sockErrToString(int32_t type) {
    switch (type) {
        case EACCES:
            return "Permission to create socket of the specified type is denied";
        case EAFNOSUPPORT:
            return "The implementation does not support the specified address family";
        case EINVAL:
            return "Invalid flags in type";
        case EMFILE:
            return "The per-process limit on the number of open file descriptors has been reached";
        case ENFILE:
            return "The system-wide limit on the total number of open files has been reached";
        case ENOBUFS:
        case ENOMEM:
            return "Insufficient memory is available.  The socket cannot be created until sufficient resources are freed";
        case EPROTONOSUPPORT:
            return "The protocol type or the specified protocol is not supported within this domain";
        default:
            return "Error type unknown";
    }
}
#else // macosx
std::string sockErrToString(int32_t type) {
    return "NEEDS IMPLEMENTATION";
}
#endif

    struct ServerStats {
        ServerStats(): msg_recvd_cnt(0),
            msg_processed_cnt(0),
            sock_err_cnt(0),
            clients_added_cnt(0),
            clients_removed_cnt(0) {}

        uint64_t msg_recvd_cnt;
        uint64_t msg_processed_cnt;
        uint64_t sock_err_cnt;
        uint64_t clients_added_cnt;
        uint64_t clients_removed_cnt;

        std::ostream& operator << (std::ostream& os) {
            os << to_string();
            return os;
        }

        std::string to_string() const {
            std::stringstream ss;
            ss << "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=Server Statistics-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n";
            ss << "\tMessages Received: " << msg_recvd_cnt << "\n";
            ss << "\tMessages Processed: " << msg_processed_cnt << "\n";
            ss << "\tClients Added: " << clients_added_cnt << "\n";
            ss << "\tClients Removed: " << clients_removed_cnt << "\n";
            ss << "\tSocket Errors: " << sock_err_cnt << "\n";
            ss << "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n";
            return ss.str();
        }
    };

    // defaults type to UDP
    struct NetConnection {
        std::string ip_addr;
        sockaddr_in sa;
        uint32_t sock_type;
        int sockfd;
        socklen_t addr_len;
        NetConnection& operator = (const NetConnection& conn) = default;
        NetConnection() : ip_addr("127.0.0.1"),
                          sa({0}),
                          sock_type(SOCK_DGRAM),
                          sockfd(INVALID_SOCKET),
                          addr_len(sizeof(sockaddr_in)) {}
        NetConnection(const NetConnection& conn) :
                            ip_addr(conn.ip_addr),
                            sa(conn.sa),
                            sock_type(conn.sock_type),
                            sockfd(conn.sockfd),
                            addr_len(sizeof(sockaddr_in)) {}
    };

    // std item to hold a buffer
    // contains socket information for easy usage
    // id :: identifies type of message being sent out || or hash_id
    // serialize() :: converts entire data structure to a buffer, reciever of this item should be able to
    //                reconstruct the message. Note serialize does not serialize NetConnection
    struct NetItem {
        NetConnection conn;
        std::vector<uint8_t> buff;

        NetItem()=default;
        NetItem(const NetItem& conn) : conn{conn.conn} { buff = conn.buff; }
        ~NetItem()=default;

        inline std::vector<uint8_t> serialize() const {
            return buff;
        }

        size_t get_buff_len() const { return buff.size(); }

        std::ostream& operator << (std::ostream& os) {
            os << "ip: " << conn.ip_addr;
            os << "port: " << conn.sa.sin_port;
            os << "buff size: " << buff.size();
            return os;
        }
    };
} // end jstd
#endif //JSTDLIB_NET_TYPES_H
