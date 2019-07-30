#include "tcp_server.h"

std::string g_ipaddr;
uint16_t g_port;

void handle_args(int argc, char** argv) {
	if (argc != 3) {
		g_port = DEFAULT_TCP_SERVER_PORT;
		g_ipaddr = LOCALHOSTIP;
		std::cout << "applying default server ipaddr: " << LOCALHOSTIP << " port: " << g_port << std::endl;
	} else {
		g_ipaddr = std::string(argv[1]);
		g_port = static_cast<uint16_t>(std::strtol(argv[2], nullptr, 10));
	}
}

int main(int argc, char** argv) {
	std::cout << "jhgfd" << std::endl;
	handle_args(argc, argv);
	jstd::net::TcpServer<jstd::net::NetItem> tcp_server(g_ipaddr, g_port);
	tcp_server.run();
	tcp_server.join_threads();
	return EXIT_SUCCESS;
}
