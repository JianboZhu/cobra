#include "cobra/acceptor.h"

#include <errno.h>
#include <fcntl.h>

#include <boost/bind.hpp>

#include "base/Logging.h"
#include "cobra/worker.h"
#include "cobra/endpoint.h"
#include "cobra/socket_wrapper.h"

namespace cobra {

Acceptor::Acceptor(Worker* loop, const Endpoint& listenAddr)
  : loop_(loop),
    accept_socket_(internal::createNonblockingOrDie()),
    accept_channel_(loop, accept_socket_.fd()),
    listenning_(false),
    idle_fd_(::open("/dev/null", O_RDONLY | O_CLOEXEC)) {
  assert(idle_fd_ >= 0);
  accept_socket_.setReuseAddr(true);
  //accept_socket_.setReusePort(reuseport);
  accept_socket_.bindAddress(listenAddr);

  // When there is a connection_request coming on the listening port,
  // call the callback function. here refers to 'Acceptor::handleRead'.
  accept_channel_.setReadCb(
      boost::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor() {
  accept_channel_.disableAll();
  accept_channel_.remove();
  ::close(idle_fd_);
}

void Acceptor::listen() {
  loop_->assertInLoopThread();
  listenning_ = true;

  // Start listening for connections on 'accept_socket_'.
  accept_socket_.listen();

  // Enable the 'read' event of the 'fd'.
  accept_channel_.enableReading();
}

void Acceptor::handleRead() {
  loop_->assertInLoopThread();

  Endpoint peer_address(0);
  //FIXME loop until no more
  int connfd = accept_socket_.accept(&peer_address);
  if (connfd >= 0) {
    if (new_conn_cb_) {
      new_conn_cb_(connfd, peer_address);
    } else {
      internal::close(connfd);
    }
  } else {
    LOG_SYSERR << "in Acceptor::handleRead";
    // Read the section named "The special problem of
    // accept()ing when you can't" in libev's doc.
    // By Marc Lehmann, author of libev.
    if (errno == EMFILE) {
      ::close(idle_fd_);
      idle_fd_ = ::accept(accept_socket_.fd(), NULL, NULL);
      ::close(idle_fd_);
      idle_fd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
    }
  }
}

}  // namespace cobra
