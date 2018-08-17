#ifndef JSTDLIB_LIBRARY_H
#define JSTDLIB_LIBRARY_H
#include <cstdint>
#include <iostream>
#include <mutex>
#include <thread>
#include <unordered_map>
#include "net_types.h"

/*
 * Description:
 *  This UDP Server will store connection information for any clients that contact it.
 *  Any connections fail to be contacted then that connection is dropped from possible
 *  clients
 *
 * make server multi-threaded with queue feeding and a processing thread
 */

namespace jstd {
    class UdpServer {
        // create a hash from ip str and and port
        std::unordered_map<size_t, NetConnection> client_connections;
    public:
        UdpServer();
        UdpServer(const std::string &ip, const int port);

        // adds udpclient to connection map
        bool addClient(const std::string, const int port);

        // virtual method to process a generic buffer
        virtual bool processMsg(uint8_t* buff);

        // broadcast message to all active clients, returns number of clients succesfully sent out to
        virtual int broadcastMsg();
    };
}
#endif
