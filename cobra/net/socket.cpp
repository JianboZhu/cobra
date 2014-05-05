#include "cobra/net/socket.h"

#include "cobra/base/Logging.h"
#include "cobra/net/inet_address.h"
#include "cobra/net/socket_wrapper.h"

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <strings.h>  // bzero

namespace cobra {
namespace net {

Socket::~Socket() {
  internal::close(sockfd_);
}

void Socket::bindAddress(const InetAddress& addr) {
  internal::bindOrDie(sockfd_, addr.getSockAddrInet());
}

void Socket::listen() {
  internal::listenOrDie(sockfd_);
}

int Socket::accept(InetAddress* peeraddr) {
  sockaddr_in addr;
  bzero(&addr, sizeof addr);
  int connfd = internal::accept(sockfd_, &addr);
  if (connfd >= 0) {
    peeraddr->setSockAddrInet(addr);
  }

  return connfd;
}

void Socket::shutdownWrite() {
  internal::shutdownWrite(sockfd_);
}

void Socket::setTcpNoDelay(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY,
               &optval, static_cast<socklen_t>(sizeof optval));
  // FIXME CHECK
}

void Socket::setReuseAddr(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR,
               &optval, static_cast<socklen_t>(sizeof optval));
  // FIXME CHECK
}

void Socket::setReusePort(bool on) {
#ifdef SO_REUSEPORT
  int optval = on ? 1 : 0;
  int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT,
                         &optval, static_cast<socklen_t>(sizeof optval));
  if (ret < 0) {
    LOG_SYSERR << "SO_REUSEPORT failed.";
  }
#else
  if (on) {
    LOG_ERROR << "SO_REUSEPORT is not supported.";
  }
#endif
}

void Socket::setKeepAlive(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE,
               &optval, static_cast<socklen_t>(sizeof optval));
  // FIXME CHECK
}

}  // namespace net
}  // namespace cobra
