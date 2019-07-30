#ifndef JSTDLIB_TCPCONNECTIONMANAGER_H
#define JSTDLIB_TCPCONNECTIONMANAGER_H
#include <unordered_map>
#include <string>
#include <mutex>
#include "net_types.h"

class TcpConnectionManager {
	std::unordered_map<uint64_t, jstd::NetConnection> m_connections;
	std::mutex m_mtx;
public:
	TcpConnectionManager()=default;
	~TcpConnectionManager()=default;

	// remove all connections in map
	void clear_connections();

	// adds new connection after successful accept
	bool add_connection(const jstd::NetConnection& conn);
	bool add_connection(jstd::NetConnection&& conn);

	// remove connection identified by ip and port, returns false if connection not found or deleted
	bool remove_connection(jstd::NetConnection& conn);

	// accepts new tcp connections adds to map
	void connection_managing();

	// return INVALID_SOCKET if not found
	int get_socket(const std::string& ipaddr, int port);
private:
	uint64_t hash_connection(const jstd::NetConnection& conn);
	uint64_t hash_connection(const std::string& ipaddr, int port);
};

#endif //JSTDLIB_TCPCONNECTIONMANAGER_H
