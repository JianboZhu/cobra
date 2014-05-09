#include "cobra/acceptor.h"

#include <errno.h>
#include <fcntl.h>

#include <boost/bind.hpp>

#include "base/Logging.h"
#include "cobra/event_loop.h"
#include "cobra/inet_address.h"
#include "cobra/socket_wrapper.h"

namespace cobra {

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr)
  : loop_(loop),
    acceptSocket_(internal::createNonblockingOrDie()),
    acceptChannel_(loop, acceptSocket_.fd()),
    listenning_(false),
    idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC)) {
  assert(idleFd_ >= 0);
  acceptSocket_.setReuseAddr(true);
  //acceptSocket_.setReusePort(reuseport);
  acceptSocket_.bindAddress(listenAddr);

  // When there is a connection_request coming on the listening port,
  // call the callback function. here refers to 'Acceptor::handleRead'.
  acceptChannel_.setReadCb(
      boost::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor() {
  acceptChannel_.disableAll();
  acceptChannel_.remove();
  ::close(idleFd_);
}

void Acceptor::listen() {
  loop_->assertInLoopThread();
  listenning_ = true;

  // Start listening for connections on 'acceptSocket_'.
  acceptSocket_.listen();

  // Enable the 'read' event of the 'fd'.
  acceptChannel_.enableReading();
}

void Acceptor::handleRead() {
  loop_->assertInLoopThread();
  InetAddress peerAddr(0);
  //FIXME loop until no more
  int connfd = acceptSocket_.accept(&peerAddr);
  if (connfd >= 0) {
    // string hostport = peerAddr.toIpPort();
    // LOG_TRACE << "Accepts of " << hostport;
    if (newConnectionCb_) {
      newConnectionCb_(connfd, peerAddr);
    } else {
      internal::close(connfd);
    }
  } else {
    LOG_SYSERR << "in Acceptor::handleRead";
    // Read the section named "The special problem of
    // accept()ing when you can't" in libev's doc.
    // By Marc Lehmann, author of libev.
    if (errno == EMFILE) {
      ::close(idleFd_);
      idleFd_ = ::accept(acceptSocket_.fd(), NULL, NULL);
      ::close(idleFd_);
      idleFd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
    }
  }
}

}  // namespace cobra
