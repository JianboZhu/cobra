// Author: Jianbo Zhu
//
// The c++ wrapper of linux c-style socket functions.

#ifndef COBRA_SOCKET_WRAPPER_H_
#define COBRA_SOCKET_WRAPPER_H_

#include <arpa/inet.h>

#include "base/basic_types.h"
#include "base/macros.h"

namespace cobra {

  class Endpoint;

  // Abort if address is in use
  void Bind(int32 listen_fd, const Endpoint& local_address);

  // Abort if address is in use
  void Listen2(int32 listen_fd);

  // On success, returns a non-negative integer that is
  // a descriptor for the accepted socket, which has been
  // set to non-blocking and close-on-exec. *peeraddr is assigned.
  // On error, -1 is returned, and *peeraddr is untouched.
  int Accept(int32 listen_fd, Endpoint* peeraddr);

  void ShutdownWrite(int32 sock_fd);

  int createNonblockingOrDie();
  int connect(int sockfd, const sockaddr_in& addr);
  ssize_t read(int sockfd, void *buf, size_t count);
  ssize_t readv(int sockfd, const iovec *iov, int iovcnt);
  ssize_t write(int sockfd, const void *buf, size_t count);
  void close(int sockfd);

  int getSocketError(int sockfd);

  sockaddr_in getLocalAddr(int sockfd);
  sockaddr_in getPeerAddr(int sockfd);

  bool isSelfConnect(int sockfd);

  /////////////////// optional settings //////////////////
  // Enable/disable TCP_NODELAY (disable/enable Nagle's algorithm).
  void SetTcpNoDelay(int32 sock_fd, bool on);

  // Enable/disable SO_REUSEADDR
  void SetReuseAddr(int32 sock_fd, bool on);

  // Enable/disable SO_REUSEPORT
  void SetReusePort(int32 sock_fd, bool on);

  // Enable/disable SO_KEEPALIVE
  void SetKeepAlive(int32 sock_fd, bool on);

}  // namespace cobra

#endif  // COBRA_SOCKET_WRAPPER_H_
