#ifndef COBRA_SOCKET_H_
#define COBRA_SOCKET_H_

#include "base/macros.h"

namespace cobra {

class Endpoint;

// Wrapper of socket file descriptor.
// It closes the sockfd when desctructs.
// It's thread safe, all operations are delagated to OS.
class Socket {
 public:
  explicit Socket(int sockfd)
    : sockfd_(sockfd) {
  }

  ~Socket();

  int fd() const { return sockfd_; }

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

#endif  // COBRA_SOCKET_H_
