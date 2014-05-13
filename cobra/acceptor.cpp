#include "cobra/acceptor.h"

#include <errno.h>
#include <fcntl.h>

#include <boost/bind.hpp>

#include "base/Logging.h"
#include "cobra/endpoint.h"
#include "cobra/socket_wrapper.h"
#include "cobra/worker.h"

namespace cobra {

Acceptor::Acceptor(Worker* loop, const Endpoint& listen_address)
  : loop_(loop),
    listen_fd_(createNonblockingOrDie()),
    accept_channel_(loop, listen_fd_),
    listenning_(false),
    idle_fd_(::open("/dev/null", O_RDONLY | O_CLOEXEC)) {
  assert(idle_fd_ >= 0);
  SetReuseAddr(listen_fd_, true);
  //SetReusePort(listen_fd_, reuseport);
  Bind(listen_fd_, listen_address);

  // When there is a connection request coming on the listening port,
  // call the callback function. here refers to 'Acceptor::handleRead'.
  accept_channel_.SetReadCb(
      boost::bind(&Acceptor::HandleRead, this));
}

Acceptor::~Acceptor() {
  accept_channel_.disableAll();
  accept_channel_.remove();
  ::close(idle_fd_);
}

void Acceptor::Listen() {
  loop_->assertInLoopThread();
  listenning_ = true;

  // Start listening for connections on 'listen_fd_'.
  Listen2(listen_fd_);

  // Enable the 'read' event of the 'fd'.
  accept_channel_.enableReading();
}

void Acceptor::HandleRead() {
  loop_->assertInLoopThread();

  Endpoint peer_address(0);
  //FIXME loop until no more
  int connfd = Accept(listen_fd_, &peer_address);
  if (connfd >= 0) {
    if (new_conn_cb_) {
      new_conn_cb_(connfd, peer_address);
    } else {
      close(connfd);
    }
  } else {
    LOG_SYSERR << "in Acceptor::handleRead";
    // Read the section named "The special problem of
    // accept()ing when you can't" in libev's doc.
    // By Marc Lehmann, author of libev.
    if (errno == EMFILE) {
      ::close(idle_fd_);
      idle_fd_ = ::accept(listen_fd_, NULL, NULL);
      ::close(idle_fd_);
      idle_fd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
    }
  }
}

}  // namespace cobra
