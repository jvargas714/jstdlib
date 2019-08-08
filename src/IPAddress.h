#ifndef JSTDLIB_IPADDRESS_H
#define JSTDLIB_IPADDRESS_H
#include <string>
#include <cstdint>
#include <vector>

/*
 * IPV4 address class that stores big-endian 4 byte network representation as well as human readable verison
 * Can construct ipv4 from a provided server/hostname
 */
namespace jstd {
    namespace net {
        class IPAddress {
            std::string ip_str;
            uint32_t ip_byte;
            std::string m_hostname;

        public:
            IPAddress();

            IPAddress(const IPAddress&);

            IPAddress(IPAddress&&) noexcept;

            IPAddress& operator = (const IPAddress& ipaddr) noexcept;
            IPAddress& operator = (IPAddress&& ipaddr) noexcept;

            explicit IPAddress(std::string ip);

            explicit IPAddress(uint32_t ip);

            inline std::string to_string() const { return ip_str; }

            inline std::string get_hostname() const { return m_hostname; }

            // return network order byte representation of ipv4 addr
            inline uint32_t operator()() const { return ip_byte; }

            friend std::ostream &operator<<(std::ostream &os, const IPAddress &ipaddr);
        };

        inline std::ostream &operator<<(std::ostream &os, const IPAddress &ipaddr) {
            os << ipaddr.to_string();
            return os;
        }
    }
}
#endif //JSTDLIB_IPADDRESS_H
