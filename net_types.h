#ifndef JSTDLIB_NET_TYPES_H
#define JSTDLIB_NET_TYPES_H
#include <cstdint>
#include <cstring>
#include <string>
#include <memory>
#include <vector>
#include <ostream>
#include <netinet/in.h>
#include <sys/socket.h>


// increment udp ports by 5
#define DEFAULT_UDP_SERVER_PORT 5005

// increment tcp ports by 2
#define DEFAULT_TCP_SERVER_PORT 5002

// localhost
#define LOCALHOSTIP "127.0.0.1"

#define INVALID_SOCKET -1

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

    // defaults type to UDP
    struct NetConnection {
        std::string ip_addr;
        sockaddr_in sa;
        uint32_t sock_type;
        int sockfd;
        socklen_t addr_len;
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

        inline std::vector<uint8_t>& serialize() {
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
