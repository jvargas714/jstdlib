#include <chrono>
#include <functional>   // std::hash
#include "logger.h"
#include "udp_server.h"
#include <arpa/inet.h>
#include <string>       // std::to_string
#include <fcntl.h>      // fcntl()
using namespace jstd;

// defines
#define LOCK_GUARD_CLIENT_MAP std::lock_guard<std::mutex> lckm(m_cmtx);
#define LOCK_GUARD_ITEM_QUEUE std::lock_guard<std::mutex> lckq(m_qmtx);
