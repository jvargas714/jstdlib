#include "TcpSocket.h"
#include <memory>
#include "net_exceptions.h"

using namespace jstd::net;

static sockaddr_in set_addr(const IPAddress& ipaddr, int port) {
    sockaddr_in addr{};
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = ipaddr();
    return addr;
}

TcpSocket::TcpSocket(std::string ipstr, int port)  :
m_ipaddr(std::move(ipstr)),
m_port(port),
m_addr{},
m_bound(false),
m_connected(false),
m_listening(false) {
    m_addr.sin_addr.s_addr = m_ipaddr();
    m_addr.sin_family = AF_INET;
    m_addr.sin_port = htons(port);
    m_sockfd = socket(AF_INET, SOCK_STREAM, 0);
}

std::ostream &jstd::net::operator<<(std::ostream &os, const TcpSocket &sock) {
    os << sock.to_string();
    return os;
}

std::string TcpSocket::to_string() const {
    std::stringstream ss;
    ss << "ip addr: " << m_ipaddr << "\n";
    ss << "port: " << m_port << "\n";
    ss << "family: AF_INET" << "\n";
    ss << "sockfd: " << m_sockfd << "\n";
    ss << "protocol: TCP";
    return ss.str();
}

bool TcpSocket::connect(const IPAddress &ip, int port) {
    if (m_connected) throw SocketConnectionException("Socket already connected");
    sockaddr_in addr = set_addr(ip, port);
    if (::connect(m_sockfd, (const sockaddr*)&addr, sizeof(addr)) < 0) {
        return false;
    }
    m_connected = true;
    return true;
}

bool TcpSocket::bind() {
    if (m_bound) throw SocketBindingException("Socket already bound");
    if (::bind(m_sockfd, (const sockaddr*)&m_addr, sizeof(sockaddr_in)) < 0) {
        return false;
    }
    m_bound = true;
    return true;
}

bool TcpSocket::listen(int backlog) {
    if (m_listening) throw SocketListeningException("Socket is already listening");
    if (::listen(m_sockfd, backlog) < 0) {
        return false;
    }
    m_listening = true;
    return true;
}

int TcpSocket::send(const std::vector<uint8_t> &data, int flags) {
    if (!m_connected) throw SocketSendingError("Socket is not connected");
    int n = ::send(m_sockfd, data.data(), data.size(), flags);
    if (n < 0) {
        throw SocketSendingError("there was an error sending data, errno: " + std::to_string(errno));
    }
    return n;
}

int TcpSocket::send(std::vector<uint8_t> &&data, int flags) {
    if (!m_connected) throw SocketSendingError("Socket is not connected");
    int n = ::send(m_sockfd, data.data(), data.size(), flags);
    if (n < 0) {
        throw SocketSendingError("there was an error sending data, errno: " + std::to_string(errno));
    }
    return n;
}

std::vector<uint8_t> TcpSocket::recv(const std::shared_ptr<TcpSocket>& from, int flags) const {
    std::vector<uint8_t> buff(MAX_BUFF_SIZE);
    int n = ::recv(from->get_fd(), buff.data(), MAX_BUFF_SIZE, flags);
    if (n < 0) {
        std::stringstream ss;
        ss << "there was an error receiving data, errno val --> " << sockErrToString(errno);
        throw SocketReceiveException(ss.str());
    }
    buff.resize(n);
    return buff;
}

TcpSocket &TcpSocket::operator=(const TcpSocket &sock) noexcept {
    m_addr = sock.m_addr;
    m_port = sock.m_port;
    m_bound = sock.m_bound;
    m_connected = sock.m_connected;
    m_bound = sock.m_bound;
    m_ipaddr = sock.m_ipaddr;
    m_listening = sock.m_listening;
    m_sockfd = sock.m_sockfd;
    return *this;
}

TcpSocket &TcpSocket::operator=(TcpSocket &&sock) noexcept {
    m_addr = sock.m_addr;
    m_port = sock.m_port;
    m_bound = sock.m_bound;
    m_connected = sock.m_connected;
    m_bound = sock.m_bound;
    m_ipaddr = std::move(sock.m_ipaddr);
    m_listening = sock.m_listening;
    m_sockfd = sock.m_sockfd;
    return *this;
}

std::shared_ptr<TcpSocket> TcpSocket::accept() const {
    if (!m_listening || !m_bound) throw SocketListeningException("Socket not listening or bound");
    sockaddr_in addr{};
    socklen_t addrlen = sizeof(sockaddr_in);
    int sockfd = ::accept(m_sockfd, (sockaddr*)&addr, &addrlen);
    if (sockfd < 0)
        throw SocketAcceptException("there was an error accepting the connecton: errno: " + std::to_string(errno));
    IPAddress ipaddr(m_addr.sin_addr.s_addr);
    return std::make_shared<TcpSocket>(ipaddr.to_string(), htons(addr.sin_port), sockfd);
}

// private, called during accept
TcpSocket::TcpSocket(std::string ipstr, int port, int sockfd):
m_sockfd(sockfd),
m_ipaddr(std::move(ipstr)),
m_port(port),
m_addr{},
m_bound(false),
m_connected(true),
m_listening(false) {
    m_addr.sin_addr.s_addr = m_ipaddr();
    m_addr.sin_family = AF_INET;
    m_addr.sin_port = htons(port);
}
