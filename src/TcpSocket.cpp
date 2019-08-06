#include "TcpSocket.h"
#include <memory>
#include "net_exceptions.h"

using namespace jstd::net;

static sockaddr_in set_addr(const IPAddress& ipaddr, int port) {
    sockaddr_in addr;
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = ipaddr();
    return addr;
}

TcpSocket::TcpSocket(std::string ipstr, int port)  :
m_ipaddr(ipstr),
m_port(port),
m_bound(false),
m_connected(false),
m_listening(false) {
    std::memset(&m_addr, 0, sizeof(sockaddr_in));
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

int TcpSocket::send(const std::vector<uint8_t> &data) {
    return 0;
}

int TcpSocket::send(const std::vector<uint8_t> &&data) {
    return 0;
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



