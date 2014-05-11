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

// Wrapper of socket file descriptor.
// It closes the sockfd when desctructs.
// It's thread safe, all operations are delagated to OS.
class Socket {
 public:
  explicit Socket(int32 sockfd)
    : sockfd_(sockfd) {
  }
  ~Socket();

  int Fd() const { return sockfd_; }

  // Abort if address is in use
  void Bind(const Endpoint& local_address);

  // Abort if address is in use
  void Listen();

  // On success, returns a non-negative integer that is
  // a descriptor for the accepted socket, which has been
  // set to non-blocking and close-on-exec. *peeraddr is assigned.
  // On error, -1 is returned, and *peeraddr is untouched.
  int Accept(Endpoint* peeraddr);

  void ShutdownWrite();

  int createNonblockingOrDie();
  int  connect(int sockfd, const sockaddr_in& addr);
  void bindOrDie(int sockfd, const sockaddr_in& addr);
  void listenOrDie(int sockfd);
  int  accept(int sockfd, sockaddr_in* addr);
  ssize_t read(int sockfd, void *buf, size_t count);
  ssize_t readv(int sockfd, const iovec *iov, int iovcnt);
  ssize_t write(int sockfd, const void *buf, size_t count);
  void close(int sockfd);
  void shutdownWrite(int sockfd);

  void toIpPort(char* buf, size_t size, const sockaddr_in& addr);
  void toIp(char* buf, size_t size, const sockaddr_in& addr);
  void fromIpPort(const char* ip, uint16_t port, sockaddr_in* addr);

  int getSocketError(int sockfd);

  sockaddr_in getLocalAddr(int sockfd);
  sockaddr_in getPeerAddr(int sockfd);

  bool isSelfConnect(int sockfd);

  /////////////////// optional settings //////////////////
  // Enable/disable TCP_NODELAY (disable/enable Nagle's algorithm).
  void SetTcpNoDelay(bool on);

  // Enable/disable SO_REUSEADDR
  void SetReuseAddr(bool on);

  // Enable/disable SO_REUSEPORT
  void SetReusePort(bool on);

  // Enable/disable SO_KEEPALIVE
  void SetKeepAlive(bool on);

 private:
  const int sockfd_;

  DISABLE_COPY_AND_ASSIGN(Socket);
};

}  // namespace cobra

#endif  // COBRA_SOCKET_WRAPPER_H_
