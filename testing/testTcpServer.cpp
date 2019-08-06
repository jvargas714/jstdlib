#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <iostream>
#include <cstdint>
#include <thread>
#include <cstring>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

constexpr uint16_t port = 5002;
constexpr char  ipaddr[] = "127.0.0.1";
constexpr uint32_t MAX_LISTEN_CNT = 100;

using std::cout;
using std::cerr;
using std::endl;

int main() {
    sockaddr_in listenAddr;
    std::memset(&listenAddr, 0, sizeof(sockaddr_in));
    listenAddr.sin_port = htons(port);
    listenAddr.sin_family = AF_INET;
    listenAddr.sin_addr.s_addr = inet_addr(ipaddr);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        cerr << "there was an error creating socket errno: " << errno << endl;
        return errno;
    }
    int rc = bind(sockfd, (const sockaddr*)&listenAddr, sizeof(sockaddr_in));
    if (rc < 0) {
        cerr << "there was an error binding errno:" << errno << endl;
        return errno;
    }
    rc = listen(sockfd, MAX_LISTEN_CNT);
    if (rc < 0) {
        cerr << "error listening, errno" << errno << endl;
        return errno;
    }

    fd_set working_set = {0};
    fd_set master_set = {0};
    FD_ZERO(&master_set);
    FD_ZERO(&working_set);
    FD_SET(sockfd, &master_set);
    int max_fd = sockfd;

    // process stuff
    while (true) {
//        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        FD_ZERO(&working_set);
        working_set = master_set;
        int n = select(max_fd+1, &working_set, NULL, NULL, NULL);
        cout << "active sock cnt: " << n << endl;
        if (n < 0) {
            cerr << "error occurred on select call" << endl;
        } else if (n == 0) {
            cerr << "sock timeout" << endl;
        } else {
            for(int fd = 0; fd <= max_fd; fd++) {
                if (FD_ISSET(fd, &working_set)) {
                    char buff[1024];
                    std::memset(buff, 0, sizeof(buff));
                    sockaddr_in addr;
                    socklen_t addrlen = sizeof(sockaddr_in);
                    if (fd == sockfd) {  // activity on listen socket new connection
                        int rc = accept(fd, (sockaddr *)&addr, &addrlen);
                        if (rc < 0) {
                            cerr << "error accepting :(" << endl;
                            exit(129);
                        } else {
                            FD_SET(rc, &master_set);
                            max_fd = std::max(rc, max_fd);
                        }
                    } else {
                        int rc = recv(fd, buff, sizeof(buff), 0);
                        if (rc < 0) {
                            cerr << "error on recv errno: " << errno << endl;
                            exit(130);
                        } else if (rc == 0) {
                            // disconnected close socket
                            FD_CLR(fd, &master_set);
                        } else {
                                cout << "read " << rc << " bytes" << endl;
                                cout << "data: " << buff << endl;
                        }
                    }
                }
            }
        }
    }
    return 0;
}
#pragma clang diagnostic pop