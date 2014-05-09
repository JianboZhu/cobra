#include "cobra/server.h"

#include <cstdio>

#include <boost/bind.hpp>

#include "base/Logging.h"
#include "cobra/acceptor.h"
#include "cobra/event_loop.h"
#include "cobra/event_loop_thread_pool.h"
#include "cobra/socket_wrapper.h"

namespace cobra {

Server::Server(EventLoop* loop,
               const InetAddress& listenAddr,
               const string& nameArg)
  : loop_(CHECK_NOTNULL(loop)),
    hostport_(listenAddr.toIpPort()),
    name_(nameArg),
    acceptor_(new Acceptor(loop, listenAddr)),
    thread_pool_(new EventLoopThreadPool(loop)),
    connectionCb_(defaultConnectionCb),
    messageCb_(defaultMessageCb),
    nextConnId_(1) {
  // Called when an connection is accpeted, @see Acceptor::HandleRead which is
  // called when a connection request is listened on 'listen fd'.
  acceptor_->setNewConnectionCb(
      boost::bind(&Server::newConnection, this, _1, _2));
}

Server::~Server() {
  loop_->assertInLoopThread();
  LOG_TRACE << "Server::~Server [" << name_ << "] dying";

  for (ConnectionMap::iterator iter = connections_.begin();
      iter != connections_.end(); ++iter) {
    TcpConnectionPtr conn = iter->second;
    iter->second.reset();
    conn->getLoop()->runInLoop(
      boost::bind(&TcpConnection::connectDestroyed, conn));
    conn.reset();
  }
}

void Server::SetThreadNum(uint32 threads) {
  assert(0 <= threads);
  thread_pool_->setThreadNum(threads);
}

void Server::start() {
  if (started_.getAndSet(1) == 0) {
    thread_pool_->start(threadInitCb_);

    assert(!acceptor_->listenning());

    // Start listening and then accepting connections right now.
    loop_->runInLoop(
        boost::bind(&Acceptor::listen, get_pointer(acceptor_)));
  }
}

// Called when an connection is established, @see Acceptor::handleRead
void Server::newConnection(int32 sockfd, const InetAddress& peerAddr) {
  loop_->assertInLoopThread();

  // This new connection is assign to a event_loop thread in a round-robin way.
  EventLoop* ioLoop = thread_pool_->getNextLoop();
  char buf[32];
  snprintf(buf, sizeof buf, ":%s#%d", hostport_.c_str(), nextConnId_);
  ++nextConnId_;
  string connName = name_ + buf;

  LOG_INFO << "Server::newConnection [" << name_
           << "] - new connection [" << connName
           << "] from " << peerAddr.toIpPort();
  InetAddress localAddr(internal::getLocalAddr(sockfd));
  // FIXME poll with zero timeout to double confirm the new connection
  // FIXME use make_shared if necessary
  TcpConnectionPtr conn(new TcpConnection(ioLoop,
                                          connName,
                                          sockfd, // the connection socket.
                                          localAddr,
                                          peerAddr));

  // Records the new connection.
  connections_[connName] = conn;

  // Set callbacks for this connection.
  conn->setConnectionCb(connectionCb_);
  conn->setMessageCb(messageCb_);
  conn->setWriteCompleteCb(writeCompleteCb_);
  conn->setCloseCb(
      boost::bind(&Server::removeConnection, this, _1)); // FIXME: unsafe

  // Run TcpConnection::connectEstablished immediately.
  ioLoop->runInLoop(boost::bind(&TcpConnection::connectEstablished, conn));
}

void Server::removeConnection(const TcpConnectionPtr& conn) {
  // FIXME: unsafe
  loop_->runInLoop(boost::bind(&Server::removeConnectionInLoop, this, conn));
}

void Server::removeConnectionInLoop(const TcpConnectionPtr& conn) {
  loop_->assertInLoopThread();
  LOG_INFO << "Server::removeConnectionInLoop [" << name_
           << "] - connection " << conn->name();
  size_t n = connections_.erase(conn->name());
  (void)n;
  assert(n == 1);
  EventLoop* ioLoop = conn->getLoop();
  ioLoop->queueInLoop(
      boost::bind(&TcpConnection::connectDestroyed, conn));
}

}  // namespace cobra
