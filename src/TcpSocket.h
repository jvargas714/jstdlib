#ifndef JSTDLIB_TCPSOCKET_H
#define JSTDLIB_TCPSOCKET_H
#include <cstdint>
#include <netinet/in.h>
#include "IPAddress.h"
#include "net_types.h"

/*
 * wrapper class around socket API
 * todo :: impl accept method that creates a new Socket object with an open sock fd, consider alternative ctor for this
 *          maybe a private ctor for this specific case
 * todo :: look in to good sockopts to set if needed or not
 * todo :: delete copy, and move ctors
 */
namespace jstd {
    namespace net {
        class TcpSocket {
            int m_sockfd;
            IPAddress m_ipaddr;
            uint16_t m_port;
            sockaddr_in m_addr;
            bool m_bound;
            bool m_connected;
            bool m_listening;

        public:
            TcpSocket() = delete;
            explicit TcpSocket(std::string ipstr, int port);
            TcpSocket(const TcpSocket&) = delete;
            TcpSocket(TcpSocket&&) = delete;
            TcpSocket& operator = (const TcpSocket& sock) noexcept;
            TcpSocket& operator = (TcpSocket&& sock) noexcept;
            virtual ~TcpSocket() = default;

            // return socket address info associated with this socket
            inline const sockaddr* operator * () const { return (const sockaddr*)(&m_addr); }

            // return socket file descriptor
            inline int operator ()() const { return m_sockfd; }
            inline int get_fd() const { return m_sockfd; }

            // connect to a server ip and port
            bool connect(const IPAddress& ip, int port);

            // bind to ipaddr and port data members
            bool bind();

            // listen for connections
            bool listen(int backlog=MAX_NUMBER_TCP_CONNECTIONS);

            // send vector of data, must be in a connected state
            int send(const std::vector<uint8_t>& data, int flags=0);
            int send(std::vector<uint8_t>&& data, int flags=0);

            // recv and return vector of data (by default BLOCKING)
            std::vector<uint8_t> recv(const std::shared_ptr<TcpSocket>& from, int flags=0) const;

            // accept new connection and return TcpSocket ptr (by default blocking)
            std::shared_ptr<TcpSocket> accept() const;

            friend std::ostream& operator << (std::ostream& os, const TcpSocket& sock);
            std::string to_string() const;

        private:
                // accept case where we create an already connected socket, should be called outside this class
                TcpSocket(std::string ipstr, int port, int sockfd);
        };

        std::ostream& operator << (std::ostream& os, const TcpSocket& sock);
    }
}

#endif //JSTDLIB_TCPSOCKET_H
