#include "cobra/endpoint.h"

#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <strings.h>  // bzero
#include <sys/socket.h>
#include <unistd.h>

#include <boost/static_assert.hpp>

#include "base/Logging.h"
#include "cobra/endian.h"
#include "cobra/socket_wrapper.h"

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

BOOST_STATIC_ASSERT(sizeof(Endpoint) == sizeof(sockaddr_in));

Endpoint::Endpoint(uint16_t port) {
  bzero(&addr_, sizeof addr_);
  addr_.sin_family = AF_INET;
  addr_.sin_addr.s_addr = hostToNetwork32(kInaddrAny);
  addr_.sin_port = hostToNetwork16(port);
}

Endpoint::Endpoint(const StringPiece& ip, uint16_t port) {
  bzero(&addr_, sizeof addr_);

  addr_.sin_family = AF_INET;
  addr_.sin_port = hostToNetwork16(port);
  if (::inet_pton(AF_INET, ip.data(), &addr_.sin_addr) <= 0) {
    LOG_SYSERR << "fromIpPort";
  }
}

string Endpoint::toIpPort() const {
  char buf[32];
  char host[INET_ADDRSTRLEN] = "INVALID";
  ::inet_ntop(AF_INET,
              &addr_.sin_addr,
              buf,
              static_cast<socklen_t>(sizeof(host)));
  uint16_t port = networkToHost16(addr_.sin_port);
  snprintf(buf, sizeof(buf), "%s:%u", host, port);

  return buf;
}

string Endpoint::toIp() const {
  char buf[32];
  size_t size = sizeof(buf);
  ::inet_ntop(AF_INET, &addr_.sin_addr, buf, static_cast<socklen_t>(size));

  return buf;
}

}  // namespace cobra
