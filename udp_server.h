#ifndef JSTDLIB_LIBRARY_H
#define JSTDLIB_LIBRARY_H
#include <cstdint>
#include <iostream>
#ifdef MULTITHREADED_SRVR
#include <mutex>
#include <thread>
#endif
#include <unordered_map>
#include <queue>
#include "net_types.h"

/*
 * Description:
 *  This UDP Server will store connection information for any clients that contact it.
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
 *      std::vector<uint8_t> data;
 *      std::vector<uint8_t> serialize();  // serialize converts data structure to bin format
 *  };
 *
 * make server multi-threaded with queue feeding and a processing thread
 */

namespace jstd {
#ifdef MULTITHREADED_SRVR
//    std::thread g_recv_thread;
//    std::thread q_proc_thread;
#endif

    template<typename QItem>
    class UdpServer {
        // create a hash from ip str and and port
        std::unordered_map<uint64_t, NetConnection> m_client_connections;

#ifdef MULTITHREADED_SRVR
        std::thread m_recv_thread;
        std::thread m_q_proc_thread;
        std::queue<std::vector<QItem>>  m_msg_queue;
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
        UdpServer();
        UdpServer(const std::string &ip, const in_port_t& port);
        ~UdpServer();

        // adds udpclient to connection map
        bool add_client(const std::string& ip, const uint16_t& port);
        void add_client(const NetConnection& conn);

        auto lookup_client(const uint64_t& hash_id, bool& found);

        // virtual method to process a QItem, hash_id of connection for response lookup
        virtual bool process_item(const QItem& item);
        virtual bool process_item(QItem&& item);
        virtual bool process_item(const QItem& item, uint64_t hash_id);

        // broadcast message to all active clients, returns number of clients succesfully sent out to
        virtual int broadcast_data(const std::vector<uint8_t>& data);

        // activate or deactivate bcast_mode
        inline bool set_bcast_mode(bool is_set);

        // set recvfrom operation to non-blocking operation
        bool set_nonblocking(bool isblocking);

        // check if non-blocking recv or not
        bool is_nonblocking();

        // clear client map
        inline void clear_clients();
        bool remove_client(const std::string& ipaddr, const int& port);

        // sends generic network message to client with hash_id
        bool send_item(const QItem& item, uint64_t hash_id);
        bool send_item(const QItem& item);

#ifdef MULTITHREADED_SRVR
        // recvs msg and queues item for processing (thread)
        void msg_recving();

        // msg processing
        void msg_processing();

        // run threads
        bool run();

        // make server run call blocking
        void join_threads();
#endif
    private:
        virtual void _build_qitem(QItem& item, const uint8_t* buff, const ssize_t& len, const sockaddr_in& addr) const;
        virtual uint64_t hash_conn(const NetConnection& conn) const;
        virtual uint64_t hash_conn(const std::string& ipaddr, const int& port) const;
        inline bool _remove_client(const std::string& ipaddr, const int& port) noexcept {
                return (m_client_connections.erase( hash_conn(ipaddr, port) ) > 0);
        }
        void push_qitem(const QItem& item);
    };
}
#endif
