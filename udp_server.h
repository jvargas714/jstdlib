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
 *  clients
 *
 *  QItem template type should have the following public interface
 *  struct QItem {
 *      int id;
 *      std::vector<uint8_t> encode();
 *  };
 *
 * make server multi-threaded with queue feeding and a processing thread
 */
class UdpServer;

namespace jstd {
#ifdef MULTITHREADED_SRVR
    std::thread g_recv_thread;
    std::thread q_proc_thread;
#endif

    template<typename QItem>
    class UdpServer {
        // create a hash from ip str and and port
        std::unordered_map<size_t, NetConnection> m_client_connections;
#ifdef MULTITHREADED_SRVR
        std::queue<std::vector<uint8_t>>  m_msg_queue;
        std::mutex m_qmtx;
        std::mutex m_cmtx;
#endif
        NetConnection m_svr_conn;

    protected:
        virtual size_t hash_conn(const NetConnection& conn) const;

    public:
        // ctors
        UdpServer();
        UdpServer(const std::string &ip, const in_port_t& port);

        // adds udpclient to connection map
        bool add_client(const std::string& ip, const uint16_t& port);
        void add_client(const NetConnection& conn);

        auto lookup_client(const uint64_t& hash_id);

        // virtual method to process a QItem, hash_id of connection for response lookup
        virtual bool process_item(const QItem& item);
        virtual bool process_item(const QItem& item, uint64_t hash_id);

        // broadcast message to all active clients, returns number of clients succesfully sent out to
        virtual int broad_cast_data(const std::vector<uint8_t>& data);

        // set recv operation to non-blocking operation
        bool set_nonblocking();

        // clear client map
        int clear_clients();

        // sends generic message to client with hash_id
        bool send_msg();

#ifdef MULTITHREADED_SRVR
        // recvs msg and queues item for processing (thread)
        void msg_recving();

        // msg processing
        void msg_processing();

        // run threads
        bool run();
#endif
    };
}
#endif
