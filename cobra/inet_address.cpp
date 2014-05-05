#include "cobra/inet_address.h"

#include "cobra/endian.h"
#include "cobra/socket_wrapper.h"

#include <strings.h>  // bzero
#include <netinet/in.h>

#include <boost/static_assert.hpp>

// INADDR_ANY use (type)value casting.
#pragma GCC diagnostic ignored "-Wold-style-cast"
static const in_addr_t kInaddrAny = INADDR_ANY;
#pragma GCC diagnostic error "-Wold-style-cast"

//     /* Structure describing an Internet socket address.  */
//      sockaddr_in {
//         sa_family_t    sin_family; /* address family: AF_INET */
//         uint16_t       sin_port;   /* port in network byte order */
//         in_addr sin_addr;   /* internet address */
//     };

//     /* Internet address. */
//     typedef uint32_t in_addr_t;
//      in_addr {
//         in_addr_t       s_addr;     /* address in network byte order */
//     };

namespace cobra {

BOOST_STATIC_ASSERT(sizeof(InetAddress) == sizeof( sockaddr_in));

InetAddress::InetAddress(uint16_t port) {
  bzero(&addr_, sizeof addr_);
  addr_.sin_family = AF_INET;
  addr_.sin_addr.s_addr = hostToNetwork32(kInaddrAny);
  addr_.sin_port = hostToNetwork16(port);
}

InetAddress::InetAddress(const StringPiece& ip, uint16_t port) {
  bzero(&addr_, sizeof addr_);
  internal::fromIpPort(ip.data(), port, &addr_);
}

string InetAddress::toIpPort() const {
  char buf[32];
  internal::toIpPort(buf, sizeof buf, addr_);
  return buf;
}

string InetAddress::toIp() const {
  char buf[32];
  internal::toIp(buf, sizeof buf, addr_);
  return buf;
}

}  // namespace cobra
