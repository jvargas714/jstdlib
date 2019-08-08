#ifndef JSTDLIB_NET_EXCEPTIONS_H
#define JSTDLIB_NET_EXCEPTIONS_H
#include <stdexcept>

namespace jstd {
    namespace net {

        class SocketConnectionException : public std::runtime_error {
        public:
            SocketConnectionException() : std::runtime_error("") {}

            explicit SocketConnectionException(const std::string &msg) : std::runtime_error(msg) {}
        };

        class SocketBindingException : public std::runtime_error {
        public:
            SocketBindingException() : std::runtime_error("") {}

            explicit SocketBindingException(const std::string &msg) : std::runtime_error(msg) {}
        };

        class SocketSendingError : public std::runtime_error {
        public:
            SocketSendingError() : std::runtime_error("") {}

            explicit SocketSendingError(const std::string &msg) : std::runtime_error(msg) {}
        };

        class SocketListeningException : public std::runtime_error {
        public:
            SocketListeningException() : std::runtime_error("") {}
            explicit SocketListeningException(const std::string& msg) : std::runtime_error(msg) {}
        };

        class SocketReceiveException : public std::runtime_error {
        public:
            SocketReceiveException() : std::runtime_error("") {}
            explicit SocketReceiveException(const std::string& msg) : std::runtime_error(msg) {}
        };

        class SocketAcceptException : public std::runtime_error {
        public:
            SocketAcceptException() : std::runtime_error("") {}
            explicit SocketAcceptException(const std::string& msg) : std::runtime_error(msg) {}
        };
    }
}

#endif //JSTDLIB_NET_EXCEPTIONS_H
