#include "cobra/socket_wrapper.h"

#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <strings.h>  // bzero
#include <sys/socket.h>
#include <unistd.h>

#include "base/Logging.h"
#include "base/Types.h"
#include "cobra/endian.h"
#include "cobra/endpoint.h"
#include "cobra/socket_wrapper.h"

namespace cobra {

namespace {

const sockaddr* sockaddr_cast(const sockaddr_in* addr) {
  return static_cast<const sockaddr*>(implicit_cast<const void*>(addr));
}

sockaddr* sockaddr_cast(sockaddr_in* addr) {
  return static_cast<sockaddr*>(implicit_cast<void*>(addr));
}

#if VALGRIND
void setNonBlockAndCloseOnExec(int sockfd) {
  // non-block
  int flags = ::fcntl(sockfd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  int ret = ::fcntl(sockfd, F_SETFL, flags);
  // FIXME check

  // close-on-exec
  flags = ::fcntl(sockfd, F_GETFD, 0);
  flags |= FD_CLOEXEC;
  ret = ::fcntl(sockfd, F_SETFD, flags);
  // FIXME check

  (void)ret;
}
#endif

}  // Anonymous namespace

Socket::~Socket() {
  close(sockfd_);
}

void Socket::Bind(const Endpoint& addr) {
  int ret = ::bind(sockfd_,
                   sockaddr_cast(&addr),
                   static_cast<socklen_t>(sizeof addr));

  if (ret < 0) {
    LOG_SYSFATAL << "bindOrDie";
  }
}

void Socket::Listen() {
  int ret = ::listen(sockfd_, SOMAXCONN);

  if (ret < 0) {
    LOG_SYSFATAL << "listenOrDie";
  }
}

int32 Socket::Accept(Endpoint* peer_addr) {
  sockaddr_in addr;
  bzero(&addr, sizeof(addr));
  socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);
#if VALGRIND
  int conn_fd = ::accept(sockfd, sockaddr_cast(addr), &addrlen);
  setNonBlockAndCloseOnExec(conn_fd);
#else
  int conn_fd = ::accept4(sockfd,
                         sockaddr_cast(addr),
                         &addrlen,
                         SOCK_NONBLOCK | SOCK_CLOEXEC);
#endif
  if (conn_fd < 0) {
    int savedErrno = errno;
    LOG_SYSERR << "Socket::accept";
    switch (savedErrno) {
      case EAGAIN:
      case ECONNABORTED:
      case EINTR:
      case EPROTO:
      case EPERM:
      case EMFILE: // per-process lmit of open file desctiptor ???
        // expected errors
        errno = savedErrno;
        break;
      case EBADF:
      case EFAULT:
      case EINVAL:
      case ENFILE:
      case ENOBUFS:
      case ENOMEM:
      case ENOTSOCK:
      case EOPNOTSUPP:
        // unexpected errors
        LOG_FATAL << "unexpected error of ::accept " << savedErrno;
        break;
      default:
        LOG_FATAL << "unknown error of ::accept " << savedErrno;
        break;
    }
  }

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

int createNonblockingOrDie() {
#if VALGRIND
  int sockfd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sockfd < 0) {
    LOG_SYSFATAL << "createNonblockingOrDie";
  }

  setNonBlockAndCloseOnExec(sockfd);
#else
  int sockfd = ::socket(
      AF_INET, // only support ipv4 now.
      SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC,
      IPPROTO_TCP); // Only support tcp now.

  if (sockfd < 0) {
    LOG_SYSFATAL << "createNonblockingOrDie";
  }
#endif
  return sockfd;
}

int connect(int sockfd, const  sockaddr_in& addr) {
  return ::connect(sockfd,
                   sockaddr_cast(&addr),
                   static_cast<socklen_t>(sizeof addr));
}

ssize_t read(int sockfd, void *buf, size_t count) {
  return ::read(sockfd, buf, count);
}

ssize_t readv(int sockfd, const  iovec *iov, int iovcnt) {
  return ::readv(sockfd, iov, iovcnt);
}

ssize_t write(int sockfd, const void *buf, size_t count) {
  return ::write(sockfd, buf, count);
}

void close(int sockfd) {
  if (::close(sockfd) < 0) {
    LOG_SYSERR << "close";
  }
}

void shutdownWrite(int sockfd) {
  if (::shutdown(sockfd, SHUT_WR) < 0) {
    LOG_SYSERR << "shutdownWrite";
  }
}

void toIpPort(char* buf,
              size_t size,
              const sockaddr_in& addr) {
  char host[INET_ADDRSTRLEN] = "INVALID";
  toIp(host, sizeof host, addr);
  uint16_t port = networkToHost16(addr.sin_port);
  snprintf(buf, size, "%s:%u", host, port);
}

void toIp(char* buf,
          size_t size,
          const  sockaddr_in& addr) {
  assert(size >= INET_ADDRSTRLEN);
  ::inet_ntop(AF_INET, &addr.sin_addr, buf, static_cast<socklen_t>(size));
}

void fromIpPort(const char* ip,
                uint16_t port,
                sockaddr_in* addr) {
  addr->sin_family = AF_INET;
  addr->sin_port = hostToNetwork16(port);
  if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0) {
    LOG_SYSERR << "fromIpPort";
  }
}

int getSocketError(int sockfd) {
  int optval;
  socklen_t optlen = static_cast<socklen_t>(sizeof optval);

  if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
    return errno;
  } else {
    return optval;
  }
}

sockaddr_in getLocalAddr(int sockfd) {
  sockaddr_in localaddr;
  bzero(&localaddr, sizeof localaddr);
  socklen_t addrlen = static_cast<socklen_t>(sizeof localaddr);
  if (::getsockname(sockfd, sockaddr_cast(&localaddr), &addrlen) < 0) {
    LOG_SYSERR << "getLocalAddr";
  }
  return localaddr;
}

sockaddr_in getPeerAddr(int sockfd) {
  sockaddr_in peeraddr;
  bzero(&peeraddr, sizeof peeraddr);
  socklen_t addrlen = static_cast<socklen_t>(sizeof peeraddr);
  if (::getpeername(sockfd, sockaddr_cast(&peeraddr), &addrlen) < 0) {
    LOG_SYSERR << "getPeerAddr";
  }
  return peeraddr;
}

bool isSelfConnect(int sockfd) {
   sockaddr_in localaddr = getLocalAddr(sockfd);
   sockaddr_in peeraddr = getPeerAddr(sockfd);
  return localaddr.sin_port == peeraddr.sin_port &&
         localaddr.sin_addr.s_addr == peeraddr.sin_addr.s_addr;
}

}  // namespace cobra
