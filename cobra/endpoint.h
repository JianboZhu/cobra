#ifndef COBRA_ENDPOINT_H_
#define COBRA_ENDPOINT_H_

#include "base/string_piece.h"

#include <netinet/in.h>

namespace cobra {

// Wrapper of sockaddr_in.
class Endpoint {
 public:
  // Cons an endpoint with given port number.
  // Mostly used in TcpServer listening.
  explicit Endpoint(uint16_t port);

  // Cons an endpoint with given ip and port.
  // @c ip should be "1.2.3.4"
  Endpoint(const StringPiece& ip, uint16_t port);

  // Cons an endpoint with given @c sockaddr_in
  // Mostly used when accepting new connections
  Endpoint(const sockaddr_in& addr)
    : addr_(addr){
  }

  string toIp() const;
  string toIpPort() const;
  string toHostPort() const __attribute__ ((deprecated))
  { return toIpPort(); }

  // default copy/assignment are Okay

  const sockaddr_in& getSockAddrInet() const { return addr_; }
  void setSockAddrInet(const sockaddr_in& addr) { addr_ = addr; }

  uint32_t ipNetEndian() const { return addr_.sin_addr.s_addr; }
  uint16_t portNetEndian() const { return addr_.sin_port; }

 private:
   sockaddr_in addr_;
};

}  // namespace cobra

#endif  // COBRA_ENDPOINT_H_
