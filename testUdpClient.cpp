#include <cstdint>
#include <cstring>
#include <iostream>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cout << "\n\nUsage: " << argv[0] << " ipaddr port\n\n" << std::endl;
        exit(-1);
    }
    struct sockaddr_in sendAddr = {0};
    socklen_t addrLen = sizeof(sockaddr_in);
    sendAddr.sin_family = AF_INET;
    sendAddr.sin_port = (uint16_t)std::strtol(argv[2], nullptr, 10);
    if(!inet_aton(argv[1], &sendAddr.sin_addr)) {
        std::cout << "invalid ip addr: " << argv[1] << " provided, aborting :(" << std::endl;
        std::cout << "errno: " << errno << std::endl;
        exit(-2);
    }

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        std::cout << "error creating socket discriptor errno: " << errno << std::endl;
        exit(-3);
    }

    // build message !!
    char buff[1024];
    memset(buff, '\0', sizeof(buff));
    strcpy(buff, "Hello from client world!!");

    int64_t len = sendto(sockfd, buff, sizeof(buff), 0, (const struct sockaddr*)&sendAddr, addrLen);
    if (len < 0) {
        std::cout << "error sending out packet to ip: " << argv[1] << ":"
                                    << argv[2] << " errno: " << errno << std::endl;
    } else {
        std::cout << "successfully sent " << len << " bytes to server: "
                                            << argv[1] << ":" << argv[2] << std::endl;
    }
    return 0;
}
