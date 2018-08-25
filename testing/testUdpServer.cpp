#include <cstdint>
#include <iostream>
#include <cstring>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <iomanip>

/*
 * test app to test sockets on unix systems
 */
static void set_nonblocking(int sockfd, bool nonblocking) {
    if (nonblocking) {
        fcntl(sockfd, F_SETFL, O_NONBLOCK);
    } else {
        if (fcntl(sockfd, F_GETFL) & O_NONBLOCK) {
            fcntl(sockfd, F_SETFL,
                  fcntl(sockfd, F_GETFL) ^ O_NONBLOCK
            );
        }
    }
}

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cout << "\n\nUsage: " << argv[0] << " ipaddr port\n\n" << std::endl;
        exit(-3);
    }

    int rc;
    auto port = (uint16_t)std::strtol(argv[2], nullptr, 10);
    struct sockaddr_in serverAddr = {0};  // this struct holds the port number and the 32bit ip addr
    struct sockaddr_in fromAddr = {0};
    socklen_t addrLen = sizeof(fromAddr);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = port;
    std::cout << "listening on port: " << serverAddr.sin_port << std::endl;

    if ( !inet_aton(argv[1], &serverAddr.sin_addr) ) {
        std::cout << "invalid ip addr string supplied, aborting :(" << std::endl;
        std::cout << "errno: " << errno << std::endl;
        exit(-1);
    }

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    rc = bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if (rc < 0) {
        std::cout << "socket failed to bind, aborting operation :(" << std::endl;
        std::cout << "errno: " << errno << std::endl;
        exit(-2);
    }

    char buff[1024];
    std::memset(buff, 0, sizeof(buff));
    std::cout << "Server with ipaddr: " << argv[1] << " listening on port " << port << std::endl;

    // set sockfd discriptor properties
    set_nonblocking(sockfd, false);

    // listen blocking, can recv from anyone
    int64_t len = recvfrom(sockfd, buff, 1024, 0, (struct sockaddr *) &fromAddr, &addrLen);
    if (len == -1) {
        std::cout << "there was an error receiving udp packets" << std::endl;
        std::cout << "errno: " << errno << std::endl;
    } else if (len == 0) {
        std::cout << "peer has performed an orderly shutdown" << std::endl;
        std::cout << "errno: " << errno << std::endl;
    } else {
        std::string tmp(buff);
        std::cout << "DATA RECVD: " << tmp << std::endl;
        std::cout << "DATA RECVD BUFF: " << buff << std::endl;
        std::cout << "from: " << inet_ntoa(fromAddr.sin_addr) << ":" << fromAddr.sin_port << std::endl;
    }
    return 0;
}

