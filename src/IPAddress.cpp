#include <arpa/inet.h>
#include <netdb.h>
#include <stdexcept>
#include "IPAddress.h"
using namespace jstd::net;

IPAddress::IPAddress(): ip_str("0.0.0.0"), ip_byte(0) { }

IPAddress::IPAddress(std::string addr): ip_str(std::move(addr)) {
    struct hostent* hent = gethostbyname(addr.c_str());
    if (!hent) {     // couldnt resolve host
        ip_byte = inet_addr(ip_str.c_str());
    } else {  // resolve host name to an ip addr
        m_hostname = hent->h_name;
        char** addlst = hent->h_addr_list;
        char* addr = addlst ? *addlst : nullptr;

        // get first valid address
        while (addr) {
            ip_byte = inet_addr(addr);
            if (ip_byte != INADDR_NONE) break;
        }
    }
    if (ip_byte == INADDR_NONE)
        throw std::runtime_error("INVALID IP DATA");
    in_addr tmp = { ip_byte };
    ip_str = inet_ntoa(tmp);
}

IPAddress::IPAddress(uint32_t ip): ip_byte(ip) {
    in_addr tmp = { ip_byte };
    ip_str = std::string(inet_ntoa(tmp));
    m_hostname = "";
}

uint32_t IPAddress::operator()() const {
    return ip_byte;
}
