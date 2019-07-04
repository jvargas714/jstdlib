#include <cstdint>
#include <cstring>
#include <iostream>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <random>
using std::string;
using std::vector;
using std::random_device;

constexpr size_t MAX_STR_LEN = 256;
constexpr size_t MSG_CNT = 100;


vector<std::string> generateRandomStrMessages(size_t cnt) {
	random_device rd;
	vector<std::string> output;
	for (int i = 0; i < cnt; i++) {
		string tmp;
		int len = rd() % MAX_STR_LEN;
		for (int j = 0; j < len; j++) {
			tmp += static_cast<char>(32 + (rd()%94) + 1);
			if (rd() % 3 == 0) {
				tmp += " ";
			} else if (rd() % 7 == 0) {
				tmp += "HOLA, GLACIAS!!!!!";
			}
		}
		output.push_back(std::move(tmp));
	}
	return output;
}

/*
 * test app to test sockets on unix systems
 */
int main(int argc, char** argv) {
    if (argc != 3) {
        std::cout << "\n\nUsage: " << argv[0] << " ipaddr port\n\n" << std::endl;
        exit(-1);
    }
    struct sockaddr_in sendAddr = {0};
    socklen_t addrLen = sizeof(sockaddr_in);
    sendAddr.sin_family = AF_INET;
    sendAddr.sin_port = (uint16_t)std::strtol(argv[2], nullptr, 10);
    std::cout << "sending to port: " << sendAddr.sin_port << std::endl;
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
    vector<std::string> msgCnt(MSG_CNT)
	for (int i = 0; i < MSG_CNT; i++) {
		int64_t len = sendto(sockfd, buff, sizeof(buff), 0, (const struct sockaddr*)&sendAddr, addrLen);
		if (len < 0) {
			std::cout << "error sending out packet to ip: " << argv[1] << ":"
			          << argv[2] << " errno: " << errno << std::endl;
		} else {
			std::cout << "successfully sent " << len << " bytes to server: "
			          << argv[1] << ":" << argv[2] << std::endl;
		}
	}

    return 0;
}
