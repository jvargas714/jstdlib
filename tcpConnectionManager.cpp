#include <functional>
#include "arpa/inet.h"
#include "tcpConnectionManager.h"
#include "logger.h"

#define TCM LOG_MODULE::TCPCONNCMGR


void TcpConnectionManager::clear_connections() {
	LOG_TRACE(TCM);
	m_connections.clear();
}

bool TcpConnectionManager::add_connection(const jstd::NetConnection& conn) {
	LOG_TRACE(TCM);
	std::hash<std::string> hasher;
	m_connections[hash_connection(conn)] = conn;
	return true;
}

bool TcpConnectionManager::add_connection(const jstd::NetConnection&& conn) {

}

bool TcpConnectionManager::remove_connection(jstd::NetConnection& conn) {
	LOG_TRACE(TCM);
	uint64_t hash = hash_connection(conn);
	if (m_connections.find(hash) != m_connections.end())
		return false;
	else
		m_connections.erase(hash);
	return true;
}

void TcpConnectionManager::connection_managing() {
	LOG_TRACE(TCM);
	while (true) {
		// todo :: manage connections !
	}
}

int TcpConnectionManager::get_socket(const std::string& ipaddr, int port) {
	LOG_TRACE(TCM);
	uint64_t hash = hash_connection(ipaddr, port);
	auto conn_entry = m_connections.find(hash);
	if (conn_entry == m_connections.end())
		return INVALID_SOCKET;
	return conn_entry->second.sockfd;
}

uint64_t TcpConnectionManager::hash_connection(const jstd::NetConnection &conn) {
	std::hash<std::string> hasher;
	return hasher(conn.ip_addr + std::to_string(conn.port));
}

uint64_t TcpConnectionManager::hash_connection(const std::string& ipaddr, int port) {
	std::hash<std::string> hasher;
	return hasher(ipaddr + std::to_string(port));
}

