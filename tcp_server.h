//
// Created by JAY VARGAS on 8/28/18.
//

#ifndef JSTDLIB_TCP_SERVER_H
#define JSTDLIB_TCP_SERVER_H
#include <cstdint>
#include <iostream>

#ifdef MULTITHREADED_SRVR
#include <mutex>
#include <thread>
#endif

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

 */
#define TSVR LOG_MODULE::TCPSERVER

namespace jstd {
	template<typename QItem>
	class TcpServer {
		// create a hash from ip str and and port
		std::unordered_map<uint64_t, NetConnection> m_client_connections;

#ifdef MULTITHREADED_SRVR
		std::thread m_recv_thread;
		std::thread m_q_proc_thread;
		std::queue<QItem> m_msg_queue;
		std::mutex m_qmtx;
		std::mutex m_cmtx;
		bool m_qproc_active;
		bool m_recv_active;
		bool m_is_nonblocking;
#endif
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
		bool add_client(const std::string &ip, const uint16_t &port);

		void add_client(const NetConnection &conn);

		auto lookup_client(const uint64_t &hash_id, bool &found);

		// virtual method to process a QItem, hash_id of connection for response lookup
		virtual bool process_item(QItem &item);

		virtual bool process_item(QItem &&item);

		virtual bool process_item(QItem &&item, uint64_t hash_id);

		// broadcast message to all active clients, returns number of clients succesfully sent out to
		virtual int broadcast_data(const std::vector<uint8_t> &data);

		// activate or deactivate bcast_mode
		inline bool set_bcast_mode(bool is_set);

		// set recvfrom operation to non-blocking operation
		bool set_nonblocking(bool isblocking);

		// check if non-blocking recv or not
		bool is_nonblocking();

		// clear client map
		inline void clear_clients();

		bool remove_client(const std::string &ipaddr, const int &port);

		// sends generic network message to client with hash_id
		bool send_item(const QItem &item);

		bool send_item(const QItem &item, uint64_t hash_id);

		// sets recv time out for blocking  recvfrom call
		bool set_recv_timeout(int milli);

#ifdef MULTITHREADED_SRVR

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

#endif
	private:
		virtual void _build_qitem(QItem &item, const uint8_t *buff, const ssize_t &len, const sockaddr_in &addr) const;

		virtual uint64_t hash_conn(const NetConnection &conn) const;

		virtual uint64_t hash_conn(const std::string &ipaddr, const int &port) const;

		inline bool _remove_client(const std::string &ipaddr, const int &port) noexcept {
			return (m_client_connections.erase(hash_conn(ipaddr, port)) > 0);
		}

		void push_qitem(const QItem &item);
	};
}



// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-Implementation=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

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
		exit(FATAL_ERR::IP_INET_FAIL);
	}
	m_svr_conn.sa.sin_family = AF_INET;
	m_svr_conn.sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (m_svr_conn.sockfd < 0) {
		LOG_ERROR(TSVR, "error creating udp socket discriptor errno # ", errno, " descr: ", sockErrToString(errno));
		exit(FATAL_ERR::SOCK_FAIL);
	}
	int rc = bind(m_svr_conn.sockfd, (const struct sockaddr *) &m_svr_conn.sa, sizeof(m_svr_conn.sa));
	if (rc < 0) {
		LOG_ERROR(TSVR, "binding socket to addr failed errno #", errno, " descr: ", sockErrToString(errno));
		exit(FATAL_ERR::SOCK_BIND_FAIL);
	}
#ifdef MULTITHREADED_SRVR
	m_is_nonblocking = false;
#endif
	LOG_INFO(TSVR, "jstd::TcpServer on IP: ", m_svr_conn.ip_addr, " port: ", m_svr_conn.sa.sin_port);
}

template<typename QItem>
jstd::TcpServer<QItem>::TcpServer(const std::string &ip, const in_port_t &port)
	: m_qproc_active(false), m_recv_active(false), m_is_bcast(false) {
	LOG_TRACE(TSVR);
	m_svr_conn.ip_addr = ip;
	m_svr_conn.sock_type = SOCK_STREAM;
	m_svr_conn.sa.sin_port = port;
	if (inet_aton(m_svr_conn.ip_addr.c_str(), &m_svr_conn.sa.sin_addr) == 0) {
		LOG_ERROR(TSVR, "invalid ip address supplied errno #", errno, " descr: ", sockErrToString(errno));
		sleep_milli(1000);
		exit(FATAL_ERR::IP_INET_FAIL);
	}
	m_svr_conn.sa.sin_family = AF_INET;
	m_svr_conn.sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (m_svr_conn.sockfd < 0) {
		LOG_ERROR(TSVR, "error creating udp socket discriptor errno # ", errno, " descr: ", sockErrToString(errno));
		sleep_milli(1000);
		exit(FATAL_ERR::SOCK_FAIL);
	}
	int rc = bind(m_svr_conn.sockfd, (const struct sockaddr *) &m_svr_conn.sa, sizeof(m_svr_conn.sa));
	if (rc < 0) {
		LOG_ERROR(TSVR, "binding socket to addr failed errno #", errno, " descr: ", sockErrToString(errno));
		sleep_milli(1000);
		exit(FATAL_ERR::SOCK_BIND_FAIL);
	}
#ifdef MULTITHREADED_SRVR
	m_is_nonblocking = false;
#endif
	LOG_INFO(TSVR, "TcpServer with IP: ", m_svr_conn.ip_addr, " port: ", m_svr_conn.sa.sin_port);
}

template<typename QItem>
jstd::TcpServer<QItem>::~TcpServer() {
	LOG_TRACE(TSVR);
	kill_threads();
	logger::get_instance().stopLogging();
}

template<typename QItem>
uint64_t jstd::TcpServer<QItem>::hash_conn(const NetConnection &conn) const {
	return hash_conn(conn.ip_addr, conn.sa.sin_port);
}

template<typename QItem>
uint64_t jstd::TcpServer<QItem>::hash_conn(const std::string &ipaddr, const int &port) const {
	std::hash<std::string> hasher;
	return hasher(ipaddr + std::to_string(port));
}

// todo :: subject to change for tcp server
template<typename QItem>
bool jstd::TcpServer<QItem>::add_client(const std::string &ip, const uint16_t &port) {
	LOG_TRACE(TSVR);
	NetConnection conn;
	conn.ip_addr = ip;
	conn.sock_type = SOCK_DGRAM;
	conn.sa.sin_port = port;
	conn.sa.sin_family = AF_INET;
	int rc = inet_aton(conn.ip_addr.c_str(), &conn.sa.sin_addr);
	if (rc < 0) {
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
	add_client(conn);
	return true;
}

// todo :: subject to change for tcp server
// add client only if not currently in map
template<typename QItem>
void jstd::TcpServer<QItem>::add_client(const NetConnection &conn) {
#ifdef MULTITHREADED_SRVR
	std::lock_guard<std::mutex> lckm(m_cmtx);
#endif
	uint64_t hash_id = hash_conn(conn);
	if (m_client_connections.find(hash_id) == m_client_connections.end())
		m_client_connections[hash_id] = conn;
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
bool jstd::TcpServer<QItem>::process_item(QItem &&item) {
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

// process item, but perform a client lookup via hash_id
// typically this can be called from the recv thread
template<typename QItem>
bool jstd::TcpServer<QItem>::process_item(QItem &&item, uint64_t hash_id) {
	bool found = false;
	auto connection = lookup_client(hash_id, found);
	if (!found) {
		LOG_WARNING(TSVR, "failed to find active connection, ", "for hash_id: ", hash_id, "aborting operation");
		return false;
	}
	item.conn = connection->second;
	return process_item(item);
}

// todo :: subject to change for tcp server
template<typename QItem>
auto jstd::TcpServer<QItem>::lookup_client(const uint64_t &hash_id, bool &found) {
#ifdef MULTITHREADED_SRVR
	std::lock_guard<std::mutex> lckm(m_cmtx);
#endif
	auto it = m_client_connections.find(hash_id);
	found = (it != m_client_connections.end());
	return it;
}

// todo :: subject to change for tcp server
template<typename QItem>
bool jstd::TcpServer<QItem>::send_item(const QItem &item) {
	LOG_TRACE(TSVR);
	std::vector<uint8_t> outBoundBuff = item.serialize();
	if (m_is_bcast) {
		int num_clients = broadcast_data(outBoundBuff);
		LOG_DEBUG(TSVR, num_clients, " have been broadcasted data");
		return true;
	}
	ssize_t bytes_sent = sendto(item.conn.sockfd,
	                            outBoundBuff.data(),
	                            outBoundBuff.size(),
	                            0,
	                            (const struct sockaddr *) &item.conn.sa,
	                            item.conn.addr_len);
	if (bytes_sent < 0) {
		LOG_ERROR(TSVR,
		          "failed to send data, errno# ",
		          errno,
		          " descr: ",
		          sockErrToString(errno));
		return false;
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

// todo :: subject to change for tcp server
template<typename QItem>
bool jstd::TcpServer<QItem>::send_item(const QItem &item, uint64_t hash_id) {
	LOG_TRACE(TSVR);
	if (m_is_bcast) {
		int num_clients = broadcast_data(item.serialize());
		LOG_DEBUG(TSVR, num_clients, " clients have been broadcasted data to");
		return true;
	}
	bool found;
	auto client_it = lookup_client(hash_id, found);
	if (!found) {
		LOG_ERROR(TSVR, "client with hash_id: ", hash_id, " not found, not sending message");
		return false;
	}
	item.conn = client_it->second;
	return send_item(item);
}

template<typename QItem>
bool jstd::TcpServer<QItem>::set_nonblocking(bool isblocking) {
	LOG_TRACE(TSVR);
	if (isblocking) {
		if (m_svr_conn.sockfd == INVALID_SOCKET) {
			LOG_ERROR(TSVR, "svr listening socket is invalid");
			return false;
		}
		fcntl(m_svr_conn.sockfd, F_SETFL, fcntl(m_svr_conn.sockfd, F_GETFL) | O_NONBLOCK);
		if (fcntl(m_svr_conn.sockfd, F_GETFL) & O_NONBLOCK) {  // todo :: this check doesnt seem to work
			LOG_WARNING(TSVR, "non blocking operation NOT set on listening socket");
			return false;
		} else {
			m_is_nonblocking = true;
			LOG_DEBUG(TSVR, "non-blocking recv operation set on listening socket");
			return true;
		}
	} else {  // set back to blocking if nonblocking already set
		if (fcntl(m_svr_conn.sockfd, F_GETFL) & O_NONBLOCK) {
			fcntl(m_svr_conn.sockfd, F_SETFL,
			      fcntl(m_svr_conn.sockfd, F_GETFL) ^ O_NONBLOCK);
			m_is_nonblocking = false;
		}
	}
	return true;
}

template<typename QItem>
bool jstd::TcpServer<QItem>::is_nonblocking() {
	return static_cast<bool>(fcntl(m_svr_conn.sockfd, F_GETFL) & O_NONBLOCK);
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
	return false;
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
#ifdef MULTITHREADED_SRVR
	std::lock_guard<std::mutex> lckm(m_cmtx);
#endif
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
#ifdef MULTITHREADED_SRVR
	std::lock_guard<std::mutex> lckm(m_cmtx);
#endif
	m_client_connections.clear();
}

// todo :: subject to change for tcp server
template<typename QItem>
bool jstd::TcpServer<QItem>::remove_client(const std::string &ipaddr, const int &port) {
	LOG_TRACE(TSVR);
	LOG_DEBUG(TSVR, "removing client ", ipaddr, ":", port);
#ifdef MULTITHREADED_SRVR
	std::lock_guard<std::mutex> lckm(m_cmtx);
#endif
	return _remove_client();
}

template<typename QItem>
void jstd::TcpServer<QItem>::_build_qitem(QItem &item,
                                          const uint8_t *buff, const ssize_t &len, const sockaddr_in &addr) const {
	item.conn.ip_addr = std::string(inet_ntoa(addr.sin_addr));
	item.conn.sa = addr;
	item.buff = std::vector<uint8_t>(buff, buff + len);
}

template<typename QItem>
bool jstd::TcpServer<QItem>::set_recv_timeout(int milli) {
	LOG_TRACE(TSVR);
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = milli * 1000;
	if (setsockopt(m_svr_conn.sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
		LOG_ERROR(TSVR, "there was an error setting socket option for recv timeout, errno", errno);
		return false;
	}
	LOG_DEBUG(TSVR, "set recv timeout to ", milli, "msec");
	return true;
}

// ------------------------------------------MULTITHREAD SUPPORT-------------------------------------------------
#ifdef MULTITHREADED_SRVR

// todo :: look in to using select or polling here
template<typename QItem>
void jstd::TcpServer<QItem>::msg_recving() {
	LOG_TRACE(TSVR);
	LOG_DEBUG(TSVR, "message receiving thread started");
	uint8_t buff[MAX_BUFF_SIZE];
	std::memset(buff, 0, MAX_BUFF_SIZE);
	ssize_t num_bytes = 0;
	sockaddr_in from_addr = {0};
	socklen_t addr_len = sizeof(sockaddr_in);
	while (m_recv_active) {
		num_bytes = recvfrom(m_svr_conn.sockfd,
		                     buff,
		                     MAX_BUFF_SIZE,
		                     0,
		                     (struct sockaddr *) &from_addr,
		                     &addr_len);
		if (num_bytes > 0) {
			QItem item;
			_build_qitem(item, buff, num_bytes, from_addr);
			LOG_INFO(TSVR, "recvd ", num_bytes, " bytes from ", item.conn.ip_addr, ":", item.conn.sa.sin_port);
			add_client(item.conn);
			push_qitem(item);
			m_stats.msg_recvd_cnt++;
			std::memset(buff, 0, MAX_BUFF_SIZE);
		}
	}
	LOG_DEBUG(TSVR, "exiting message recv thread...");
}

template<typename QItem>
void jstd::TcpServer<QItem>::msg_processing() {
	LOG_TRACE(TSVR);
	LOG_DEBUG(TSVR, "message processing thread started");
	while (m_qproc_active) {
		sleep_milli(DEFAULT_SVR_THREAD_SLEEP);
		if (!m_msg_queue.empty()) {
			if (process_item(std::move(m_msg_queue.front())))
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
	LOG_DEBUG(TSVR, "UDP server is now blocking, until app termination");
	m_recv_thread.join();
	m_q_proc_thread.join();
	LOG_DEBUG(TSVR, "UDP server theads have exited");
}

template<typename QItem>
void jstd::TcpServer<QItem>::kill_threads() {
	LOG_TRACE(TSVR);
	LOG_DEBUG(TSVR, "shuttdown server threads");
	LOG_DEBUG(TSVR, "\n", m_stats);
	m_qproc_active = false;
	m_recv_active = false;
	join_threads();
}

#endif    // THREADED REGION OF SOURCE
#endif //JSTDLIB_TCP_SERVER_H
