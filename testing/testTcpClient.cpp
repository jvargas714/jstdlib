#include <cstdint>
#include <cstring>
#include <iostream>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <random>
#include <sstream>

using std::string;
using std::vector;
using std::random_device;
using std::endl;
using std::cerr;
using std::cin;
using std::cout;

constexpr size_t MSG_CNT = 10000;
const string MSG = "HELLO FROM TCP CLIENT!!!!!!!!!!!!!!";

static std::vector<std::string> generateTestStrings() {
	vector<string> msgs;
	for (size_t i = 1; i < MSG_CNT; i++)
		msgs.emplace_back(std::to_string(i) + " >> " + MSG);
	return msgs;
}

int main(int argc, char* argv[]) {
	if (argc != 3) {
		std::cerr << "invalid args were provided, try again" << std::endl;
		return 127;
	}
	struct sockaddr_in sendAddr = {};
	std::memset(&sendAddr, 0, sizeof(sockaddr_in));
	const socklen_t addrLen = sizeof(sockaddr_in);
	sendAddr.sin_family = AF_INET;
	cout << "attempting to connect to " << argv[1] << ":" << argv[2] << endl;
	uint16_t port = (uint16_t)std::strtol(argv[2], nullptr, 10);
	sendAddr.sin_port = htons((uint16_t)port);
	std::cout << "sending to port: " << sendAddr.sin_port << std::endl;
	if(!inet_aton(argv[1], &sendAddr.sin_addr)) {
		std::cout << "invalid ip addr: " << argv[1] << " provided, aborting :(" << std::endl;
		std::cout << "errno: " << errno << std::endl;
		exit(-2);
	}

	// create socket
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		cerr << "there was an error creating socket, exiting" << endl;
		exit(-3);
	}

	// connect to server
	if (connect(sockfd, (const sockaddr*)&sendAddr, addrLen) < 0) {
		cerr << "error connecting to server, exiting :(" << endl;
		exit(-4);
	}

	// send data to server
	auto msgs = generateTestStrings();
	cout << "----------SENDING DATA---------\n" << endl;
	char resp = 'y';
	string pref;
	uint64_t numBytes = 0;
	int rc = 0;
	cout << "ready to send data (y/n)?" << endl;
	cin >> resp;
	while (resp == 'y') {
		cout << "send one or many messages (one, many)" << endl;
		cin >> pref;
		if (pref == "many") {
			for (const auto &msg : msgs) {
				cout << "sending " << msg.length() << " bytes" << endl;
				rc = send(sockfd, (char *) msg.c_str(), strlen(msg.c_str()), 0);
				if (rc < 0) {
					cerr << "there was an error sending data to server, exiting :(" << endl;
					exit(-5);
				}
			}
		} else {
			char buff[] = "this is a single message!!!!!";
			rc = send(sockfd, (char *) buff, strlen(buff), 0);
			if (rc < 0) {
				cerr << "there was an error sending data to server, exiting :(" << endl;
				exit(-5);
			}
		}
		numBytes += rc;
		cout << "successfully sent " << rc << " bytes" << endl;
		cout << "total bytes sent: " << numBytes << ". Send another round of messages? (y/n)" << endl;
		cin >> resp;
	}
	return 0;
}
