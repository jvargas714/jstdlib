#ifndef JSTDLIB_TCP_SERVER_H
#define JSTDLIB_TCP_SERVER_H
#include <stdlib.h>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <queue>
#include <chrono>
#include <functional>   // std::hash
#include "logger.h"
#include "udp_server.h"
#include <arpa/inet.h>
#include <string>       // std::to_string
#include <fcntl.h>      // fcntl()
#include "jstd_util.h"
#include <sys/filio.h>
#include <sys/ioctl.h>  // ioctl()
#include <unistd.h>     // close()
#include "net_types.h"

/*
 * Description:
 *  This TCP Server will store connection information for any clients that contact it.
 *  Any connections fail to be contacted then that connection is dropped from possible
 *  clients map.
 *
 *  Broadcast mode can be enabled. This is not true UDP bcast as all active clients will recv the same message.
 *  This makes having bcast/multicast routers not a requirement
 *
 *  This Server can also be built SINGLE or MULTITHREADED
 *
 *  QItem template type should have the following public interface
 *  struct QItem {
 *      NetConnection conn;
 *      fields...
 *      fields...
 *      fields...
 *      std::vector<uint8_t> serialize();  // serialize converts data structure to bin format
 *  };

 ISSUES:
 todo :: having issues with the timeout value set to other than nullptr
 */
#define TSVR LOG_MODULE::TCPSERVER

namespace jstd {
	template<typename QItem>
	class TcpServer {
		// create a hash from ip str and and port
		std::unordered_map<uint64_t, NetConnection> m_client_connections;

		std::thread m_recv_thread;
		std::thread m_q_proc_thread;
		std::queue<QItem> m_msg_queue;
		std::mutex m_qmtx;
		std::mutex m_cmtx;
		bool m_qproc_active;
		bool m_recv_active;
		FdSets m_fd_sets;

		// listening socket
		NetConnection m_svr_conn;

		// broadcast mode flag
		bool m_is_bcast;

		// message counter
		ServerStats m_stats;

	public:
		// ctors
		TcpServer();
		TcpServer(const std::string &ip, const in_port_t &port);
		~TcpServer();

		// adds udpclient to connection map
		void add_client(const NetConnection &conn);
		bool add_client(const std::string &ip, const uint16_t &port);

		// find client by socket descriptor
		auto lookup_client(const std::string& ipaddr, const in_port_t& port);
		auto lookup_client(int sockfd);

		// remove client identified by
		bool remove_client(const std::string& ipaddr, const in_port_t& port);
		bool remove_client(const NetConnection& conn);

		// process methods
		virtual bool process_item(QItem &item);
		virtual bool process_item(QItem &&item);
		virtual bool process_select_timeout();

		// broadcast message to all active clients, returns number of clients succesfully sent out to
		virtual int broadcast_data(const std::vector<uint8_t> &data);

		// activate or deactivate bcast_mode
		inline bool set_bcast_mode(bool is_set);

		// clear client map
		inline void clear_clients();

		// sends generic network message to client with hash_id
		bool send_item(const QItem &item);

		// send message to connection associated with the socket descriptor
		bool send_item(const QItem &item, const std::string& ipaddr, const in_port_t& port);

		// sets recv time out for blocking  recvfrom call
		bool set_recv_timeout(int milli);

		// process data from associated connection
		virtual void on_data(std::vector<uint8_t>&& data, const NetConnection& conn);

		// recvs msg and queues item for processing (thread)
		void msg_recving();

		// msg processing
		void msg_processing();

		// run threads
		bool run();

		// make server run call blocking
		void join_threads();

		// kill and join threads
		void kill_threads();

	private:
		virtual QItem build_qitem(std::vector<uint8_t>&& data, const NetConnection& conn) const;
		virtual uint64_t hash_conn(const NetConnection &conn) const;
		virtual uint64_t hash_conn(const std::string &ipaddr, const int &port) const;
		inline bool _remove_client(const std::string &ipaddr, const int &port) noexcept;
		bool init_listen_socket();
		void push_qitem(const QItem &item);
		int select_active_socket();
		bool accept_new_connection(int sockfd);
		void recv_data(int sockfd);
		virtual void handle_select_error();
	};
}



// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-Implementation=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
template<typename QItem>
bool jstd::TcpServer<QItem>::init_listen_socket() {
	LOG_DEBUG(TSVR, "initializing listener socket");
	int on;
	int rc = bind(m_svr_conn.sockfd, (const struct sockaddr *) &m_svr_conn.sa, sizeof(m_svr_conn.sa));
	if (rc < 0) {
		LOG_ERROR(TSVR, "binding socket to addr failed errno #", errno, " descr: ", sockErrToString(errno));
		return false;
	}
	rc = listen(m_svr_conn.sockfd, 100);
	if (rc < 0) {
		LOG_ERROR(TSVR, "there was an error listening on socket, exiting errno: ",
			errno, " descr: ", sockErrToString(errno));
		return false;
	}
	rc = setsockopt(m_svr_conn.sockfd, SOL_SOCKET,  SO_REUSEADDR, (char *)&on, sizeof(on));
	if (rc < 0) {
		LOG_ERROR(TSVR, "setting socket options on listening socket failed");
		close(m_svr_conn.sockfd);
		return false;
	}
	// set socket to non-blocking
	rc = ioctl(m_svr_conn.sockfd, FIONBIO, (char *)&on);
	if (rc < 0) {
		LOG_ERROR(TSVR, "ioctl() failed");
		close(m_svr_conn.sockfd);
		return false;
	}
	// default TIME OUT
	set_recv_timeout(DEFAULT_TCP_RECV_TIMEOUT_MILLI);
	return true;
}

// default connection settings
template<typename QItem>
jstd::TcpServer<QItem>::TcpServer()
	: m_qproc_active(false), m_recv_active(false), m_is_bcast(false) {
	LOG_TRACE(TSVR);
	m_svr_conn.ip_addr = LOCALHOSTIP;
	m_svr_conn.sock_type = SOCK_STREAM;
	m_svr_conn.sa.sin_port = DEFAULT_TCP_SERVER_PORT;
	if (!inet_aton(LOCALHOSTIP, &m_svr_conn.sa.sin_addr)) {
		LOG_ERROR(TSVR, "invalid ip address supplied errno #", errno, " descr: ", sockErrToString(errno));
		std::exit((static_cast<int>(FATAL_ERR::IP_INET_FAIL)));
	}
	m_svr_conn.sa.sin_family = AF_INET;
	m_svr_conn.sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (m_svr_conn.sockfd < 0) {
		LOG_ERROR(TSVR, "error creating udp socket discriptor errno # ", errno, " descr: ", sockErrToString(errno));
		std::exit((static_cast<int>(FATAL_ERR::SOCK_FAIL)));
	}
	if (!init_listen_socket()) {
		LOG_ERROR(TSVR, "There was an error listening ");
		std::exit((static_cast<int>(FATAL_ERR::SOCK_LISTEN_FAIL)));
	}
}

template<typename QItem>
jstd::TcpServer<QItem>::TcpServer(const std::string &ip, const in_port_t &port)
	: m_qproc_active(false), m_recv_active(false), m_is_bcast(false) {
	LOG_TRACE(TSVR);
	m_svr_conn.ip_addr = ip;
	m_svr_conn.sock_type = SOCK_STREAM;
	m_svr_conn.sa.sin_port = htons(port);
	if (inet_aton(m_svr_conn.ip_addr.c_str(), &m_svr_conn.sa.sin_addr) == 0) {
		LOG_ERROR(TSVR, "invalid ip address supplied errno #", errno, " descr: ", sockErrToString(errno));
		sleep_milli(1000);
		std::exit((static_cast<int>(FATAL_ERR::IP_INET_FAIL)));
	}
	m_svr_conn.sa.sin_family = AF_INET;
	m_svr_conn.sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (m_svr_conn.sockfd < 0) {
		LOG_ERROR(TSVR, "error creating udp socket discriptor errno # ", errno, " descr: ", sockErrToString(errno));
		sleep_milli(1000);
		std::exit((static_cast<int>(FATAL_ERR::SOCK_FAIL)));
	}
	if (!init_listen_socket()) {
		LOG_ERROR(TSVR, "There was an error listening ");
		std::exit((static_cast<int>(FATAL_ERR::SOCK_LISTEN_FAIL)));
	}
}

template<typename QItem>
jstd::TcpServer<QItem>::~TcpServer() {
	LOG_TRACE(TSVR);
	kill_threads();
	logger::get_instance().stopLogging();
}

template<typename QItem>
uint64_t jstd::TcpServer<QItem>::hash_conn(const NetConnection &conn) const {
	std::hash<int> hasher;
	return hasher(conn.sockfd);
}

template<typename QItem>
uint64_t jstd::TcpServer<QItem>::hash_conn(const std::string &ipaddr, const int &port) const {
	std::hash<std::string> hasher;
	return hasher(ipaddr + std::to_string(port));
}

template<typename QItem>
bool jstd::TcpServer<QItem>::add_client(const std::string &ip, const uint16_t &port) {
	LOG_TRACE(TSVR);
	NetConnection conn;
	conn.ip_addr = ip;
	conn.sock_type = SOCK_STREAM;
	conn.sa.sin_port = port;
	conn.sa.sin_family = AF_INET;
	if (inet_aton(conn.ip_addr.c_str(), &conn.sa.sin_addr) < 0) {
		LOG_ERROR(TSVR, "there was an error converting ip str:", ip, " to binary form");
		return false;
	}
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		LOG_ERROR(TSVR,
		          "there was an error creating sockect discriptor, eerno: #",
		          errno,
		          " descr: ",
		          sockErrToString(errno));
		return false;
	}
	conn.sockfd = sockfd;
	add_client(conn);
	return true;
}

// add client only if not currently in map, overwrites if on same ip and port
template<typename QItem>
void jstd::TcpServer<QItem>::add_client(const NetConnection &conn) {
	std::lock_guard<std::mutex> lckm(m_cmtx);
	LOG_DEBUG(TSVR, "adding new client with ip: ", conn.ip_addr, " port: ", conn.port);
	m_client_connections[hash_conn(conn)] = conn;
	m_stats.clients_added_cnt++;
}

// process item off the msg queue
// assumes item has valid connection information
template<typename QItem>
bool jstd::TcpServer<QItem>::process_item(QItem &item) {
	LOG_TRACE(TSVR);
	LOG_INFO(TSVR, "processing item recvd:\n", item);
	std::stringstream ss;
	for (const auto &el : item.buff) ss << el;
	LOG_INFO(TSVR, "data recvd: ", ss.str());
	std::string tmp_msg = "hello thanks for the message, unfortunately this Server does nothing, IMPLEMENT ME!!\n";
	QItem resp;
	resp.buff = std::vector<uint8_t>(tmp_msg.begin(), tmp_msg.end());
	resp.conn = item.conn;
	return send_item(resp);
}

template<typename QItem>
bool jstd::TcpServer<QItem>::process_item(QItem&& item) {
	LOG_TRACE(TSVR);
	LOG_INFO(TSVR, "processing rval ref item recvd:\n", item);
	std::stringstream ss;
	for (const auto &el : item.buff) ss << el;
	LOG_INFO(TSVR, "data recvd: ", ss.str());
	std::string tmp_msg = "hello thanks for the message, unfortunately this Server does nothing, IMPLEMENT ME!!\n";
	QItem resp;
	resp.buff = std::vector<uint8_t>(tmp_msg.begin(), tmp_msg.end());
	resp.conn = item.conn;
	return send_item(resp);
}

template<typename QItem>
bool jstd::TcpServer<QItem>::send_item(const QItem &item) {
	LOG_TRACE(TSVR);
	std::vector<uint8_t> outBoundBuff = item.serialize();
	if (m_is_bcast) {
		int num_clients = broadcast_data(outBoundBuff);
		LOG_DEBUG(TSVR, num_clients, " have been broadcasted data");
		return true;
	}
	ssize_t bytes_sent = send(item.conn.sockfd, outBoundBuff.data(), outBoundBuff.size(), 0);
	if (bytes_sent == SOCKET_ERROR) {
		LOG_ERROR(TSVR,
		          "failed to send data, errno# ",
		          errno,
		          " descr: ",
		          sockErrToString(errno));
		return false;
	} else if (bytes_sent == 0) {
		LOG_WARNING(TSVR, "client is no longer connected, removing connection from client map :(");
		LOG_DEBUG(TSVR, (remove_client(item.conn)) ? "successfully removed client":"failed to remove client");
	}
	LOG_INFO(TSVR,
	         "successfully sent out ",
	         bytes_sent,
	         " bytes of data to ",
	         item.conn.ip_addr,
	         ":",
	         item.conn.sa.sin_port);
	return true;
}

template<typename QItem>
bool jstd::TcpServer<QItem>::send_item(const QItem &item, const std::string& ipaddr, const in_port_t& port) {
	LOG_TRACE(TSVR);
	if (m_is_bcast) {
		int num_clients = broadcast_data(item.serialize());
		LOG_DEBUG(TSVR, num_clients, " clients have been broadcasted data to");
		return true;
	}
	auto client_it = lookup_client(ipaddr, port);
	if (client_it == m_client_connections.end()) {
		LOG_ERROR(TSVR, "client not found, not sending message");
		return false;
	}
	item.conn = client_it->second;
	return send_item(item);
}

template<typename QItem>
bool jstd::TcpServer<QItem>::set_bcast_mode(bool is_set) {
	LOG_TRACE(TSVR);
	if (is_set) {
		LOG_INFO(TSVR, "setting server to broadcast mode, current client count: ",
		         m_client_connections.size());
	} else {
		LOG_INFO(TSVR, "disabling broadcast mode on server");
	}
	m_is_bcast = is_set;
	return is_set;
}

template<typename QItem>
int jstd::TcpServer<QItem>::broadcast_data(const std::vector<uint8_t> &data) {
	LOG_TRACE(TSVR);
	if (data.empty()) {
		LOG_WARNING(TSVR, "data buffer empty, not bcasting data");
		return 0;
	}
	QItem item;
	int client_cnt = 0;
	item.buff = data;
	LOG_DEBUG(TSVR, "broadcasting data to ", m_client_connections.size(), " clients");
	std::lock_guard<std::mutex> lckm(m_cmtx);
	for (const auto &client : m_client_connections) {
		item.conn = client.second;
		if (send_item(item)) {
			client_cnt++;
		} else {
			LOG_WARNING(TSVR, "removing client: ", item.conn.ip_addr, ":", item.conn.sa.sin_port);
			_remove_client(item.conn.ip_addr, item.conn.sa.sin_port);
		}
	}
	LOG_DEBUG(TSVR, "successfully sent data to ",
	          client_cnt, "/", m_client_connections.size(), " clients");
	return client_cnt;
}

template<typename QItem>
void jstd::TcpServer<QItem>::clear_clients() {
	std::lock_guard<std::mutex> lckm(m_cmtx);
	m_client_connections.clear();
}

template<typename QItem>
QItem jstd::TcpServer<QItem>::build_qitem(std::vector<uint8_t>&& data, const NetConnection& conn) const {
	QItem item;
	item.conn = conn;
	item.buff = std::move(data);
	return item;
}

template<typename QItem>
bool jstd::TcpServer<QItem>::set_recv_timeout(int milli) {
	LOG_TRACE(TSVR);
	// if zero timeout val then no timeout
	m_fd_sets.timeout.tv_sec = 0;
	m_fd_sets.timeout.tv_usec = milli * 1000;
	LOG_DEBUG(TSVR, "set recv timeout to ", milli, "msec");
	return true;
}

template<typename QItem>
void jstd::TcpServer<QItem>::msg_recving() {
	LOG_TRACE(TSVR);
	LOG_DEBUG(TSVR, "message receiving thread started");
	uint8_t buff[MAX_BUFF_SIZE];
	std::memset(buff, 0, MAX_BUFF_SIZE);
	ssize_t len = 0;
	sockaddr_in from_addr = {0};
	socklen_t addr_len = sizeof(sockaddr_in);
	FD_ZERO(&m_fd_sets.master_set);
	FD_SET(m_svr_conn.sockfd, &m_fd_sets.master_set);
	m_fd_sets.max_fd = m_svr_conn.sockfd;
	int sockfd;
	while (m_recv_active) {
		sockfd = select_active_socket();
		if (sockfd == m_svr_conn.sockfd) // listener socket is active
			accept_new_connection(sockfd);
		else if (sockfd == SELECT_TIMEOUT)
			process_select_timeout();
		else if (sockfd == SOCKET_ERROR)
			handle_select_error();
		else
			recv_data(sockfd);
	}
	LOG_DEBUG(TSVR, "exiting message recv thread...");
}

template<typename QItem>
void jstd::TcpServer<QItem>::recv_data(int sockfd) {
	LOG_TRACE(TSVR);
	uint8_t buff[MAX_BUFF_SIZE];
	ssize_t len = recv(sockfd, buff, MAX_BUFF_SIZE, 0);
	if (len > 0) {
		auto conn_entry = lookup_client(sockfd);
		if (conn_entry == m_client_connections.end()) {
			LOG_WARNING(TSVR, "connection associated with recvd data not found, not processing data");
			return;
		}
		on_data(std::vector<uint8_t>(buff, buff + len), conn_entry->second);
	} else if (len == 0) {
		LOG_DEBUG(TSVR, "connection has been closed by client");
	} else if (len == SOCKET_ERROR) {
		LOG_ERROR(TSVR, "an error occured receiving data :( errno: ", errno, " descr: ", sockErrToString(errno));
	}
	else {
		LOG_WARNING(TSVR, "socket recv state unknown ??");
	}
}

template<typename QItem>
void jstd::TcpServer<QItem>::on_data(std::vector<uint8_t>&& data, const NetConnection& conn) {
	LOG_DEBUG(TSVR, "building qitem for processing. A ", data.size(), " byte tcp packet");
	auto item = build_qitem(std::move(data), conn);
	push_qitem(item);
}

template<typename QItem>
void jstd::TcpServer<QItem>::msg_processing() {
	LOG_TRACE(TSVR);
	LOG_DEBUG(TSVR, "message processing thread started");
	while (m_qproc_active) {
		sleep_milli(DEFAULT_SVR_THREAD_SLEEP);
		if (!m_msg_queue.empty()) {
			if ( process_item(std::move(m_msg_queue.front())) )
				m_stats.msg_processed_cnt++;
			m_qmtx.lock();
			m_msg_queue.pop();
			m_qmtx.unlock();
		}
	}
	LOG_DEBUG(TSVR, "terminating message processing thread");
}

template<typename QItem>
bool jstd::TcpServer<QItem>::run() {
	LOG_DEBUG(TSVR, "starting message receiving and item processing thread");
	m_qproc_active = true;
	m_recv_active = true;
	m_recv_thread = std::thread(&TcpServer::msg_recving, this);
	m_q_proc_thread = std::thread(&TcpServer::msg_processing, this);
	return true;
}

template<typename QItem>
void jstd::TcpServer<QItem>::push_qitem(const QItem &item) {
	LOG_TRACE(TSVR);
	std::lock_guard<std::mutex> lckm(m_qmtx);
	m_msg_queue.push(item);
}

template<typename QItem>
void jstd::TcpServer<QItem>::join_threads() {
	LOG_TRACE(TSVR);
	LOG_DEBUG(TSVR, "server is now blocking, until app termination");
	m_recv_thread.join();
	m_q_proc_thread.join();
	LOG_DEBUG(TSVR, "server threads have exited...");
}

template<typename QItem>
void jstd::TcpServer<QItem>::kill_threads() {
	LOG_TRACE(TSVR);
	LOG_DEBUG(TSVR, "shuttdown server threads");
	LOG_DEBUG(TSVR, "\n", m_stats, "\n");
	m_qproc_active = false;
	m_recv_active = false;
	join_threads();
}

// function selects the active socket descriptor from the master sock fd list and returns it
template<typename QItem>
int jstd::TcpServer<QItem>::select_active_socket() {
	LOG_TRACE(TSVR);
	std::memcpy(&m_fd_sets.working_set, &m_fd_sets.master_set, sizeof(m_fd_sets.master_set));
	int rc = select(m_fd_sets.max_fd+1, &m_fd_sets.working_set, nullptr, nullptr, nullptr);
	if (rc == SOCKET_ERROR) return SOCKET_ERROR;
	if (rc == 0) return SELECT_TIMEOUT;
	for (int i = 0; i < m_fd_sets.max_fd; i++) {
		if (FD_ISSET(i, &m_fd_sets.working_set)) {
			return i;
		}
	}
	return SOCKET_ERROR;
}

template<typename QItem>
bool jstd::TcpServer<QItem>::accept_new_connection(int sockfd) {
	LOG_TRACE(TSVR);
	LOG_DEBUG(TSVR, "accepting new connections");
	int new_sd;
	int cnt = 0;
	NetConnection new_conn;
	while(true) {
		new_sd = accept(sockfd, (struct sockaddr*)&new_conn.sa, &new_conn.addr_len);
		if (new_sd != SOCKET_ERROR) {
			new_conn.ip_addr = std::string(inet_ntoa(new_conn.sa.sin_addr));
			new_conn.port = ntohl(new_conn.sa.sin_port);
			new_conn.sockfd = sockfd;
			new_conn.sock_type = SOCK_STREAM;
			FD_SET(new_sd, &m_fd_sets.master_set);
			if (new_sd > m_fd_sets.max_fd)
				m_fd_sets.max_fd = new_sd;
			add_client(new_conn);
			cnt++;
		} else
			break;
	}
	LOG_DEBUG(TSVR, "added ", cnt, " new connections");
	return cnt>0;
}

template<typename QItem>
inline bool jstd::TcpServer<QItem>::_remove_client(const std::string &ipaddr, const int &port) noexcept {
	std::lock_guard<std::mutex> lckm(m_cmtx);
	return remove_client(ipaddr, port);
}

template<typename QItem>
auto jstd::TcpServer<QItem>::lookup_client(const std::string &ipaddr, const in_port_t &port) {
	LOG_TRACE(TSVR);
	std::lock_guard<std::mutex> lck(m_cmtx);
	LOG_DEBUG(TSVR, "performing client lookup with ip: ", ipaddr, " and port: ", port);
	return m_client_connections.find(hash_conn(ipaddr, port));
}

template<typename QItem>
bool jstd::TcpServer<QItem>::remove_client(const std::string &ipaddr, const in_port_t &port) {
	LOG_DEBUG(TSVR, "removing client connection ipaddr: ", ipaddr, " and port: ", port);
	std::lock_guard<std::mutex> lck(m_qmtx);
	size_t cnt = m_client_connections.erase(hash_conn(ipaddr, port));
	return cnt > 0;
}

template<typename QItem>
bool jstd::TcpServer<QItem>::remove_client(const jstd::NetConnection &conn) {
	LOG_TRACE(TSVR);
	return remove_client(conn.ip_addr, static_cast<const in_port_t>(conn.port));
}

template<typename QItem>
auto jstd::TcpServer<QItem>::lookup_client(int sockfd) {
	struct sockaddr_in addr = {0};
	socklen_t len = sizeof(sockaddr_in);
	int rc = getpeername(sockfd, (struct sockaddr*)&addr, &len);
	if (rc == SOCKET_ERROR) {
		LOG_ERROR(TSVR, "there was an error getting the socket address. errno: ",
			errno, " descr: ", sockErrToString(errno));
		return m_client_connections.end();
	}
	return lookup_client(std::string(inet_ntoa(addr.sin_addr)), ntohs(addr.sin_port));
}

template<typename QItem>
bool jstd::TcpServer<QItem>::process_select_timeout() {
	LOG_TRACE(TSVR);
	LOG_DEBUG(TSVR, "processing select timout event!!!");
	return true;
}

template<typename QItem>
void jstd::TcpServer<QItem>::handle_select_error() {
	LOG_TRACE(TSVR);
	LOG_DEBUG(TSVR, "handling select error...");
}

#endif //JSTDLIB_TCP_SERVER_H
