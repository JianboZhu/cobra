#include "cobra/socket.h"

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <strings.h>  // bzero

#include "base/Logging.h"
#include "cobra/endpoint.h"
#include "cobra/socket_wrapper.h"

namespace cobra {

Socket::~Socket() {
  internal::close(sockfd_);
}

void Socket::Bind(const Endpoint& addr) {
  internal::bindOrDie(sockfd_, addr.getSockAddrInet());
}

void Socket::Listen() {
  internal::listenOrDie(sockfd_);
}

int Socket::Accept(Endpoint* peer_addr) {
  sockaddr_in addr;
  bzero(&addr, sizeof(addr));
  int conn_fd = internal::accept(sockfd_, &addr);
  if (conn_fd >= 0) {
    peer_addr->setSockAddrInet(addr);
  }

  return conn_fd;
}

void Socket::ShutdownWrite() {
  internal::shutdownWrite(sockfd_);
}

void Socket::SetTcpNoDelay(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY,
               &optval, static_cast<socklen_t>(sizeof optval));
  // FIXME CHECK
}

void Socket::SetReuseAddr(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR,
               &optval, static_cast<socklen_t>(sizeof optval));
  // FIXME CHECK
}

void Socket::SetReusePort(bool on) {
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

void Socket::SetKeepAlive(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE,
               &optval, static_cast<socklen_t>(sizeof optval));
  // FIXME CHECK
}

}  // namespace cobra
