#include "cobra/tcp_server.h"

#include <cstdio>

#include <boost/bind.hpp>

#include "base/Logging.h"
#include "cobra/acceptor.h"
#include "cobra/event_loop.h"
#include "cobra/event_loop_thread_pool.h"
#include "cobra/socket_wrapper.h"

namespace cobra {

TcpServer::TcpServer(EventLoop* loop,
                     const InetAddress& listenAddr,
                     const string& nameArg,
                     Option option)
  : loop_(CHECK_NOTNULL(loop)),
    hostport_(listenAddr.toIpPort()),
    name_(nameArg),
    acceptor_(new Acceptor(loop, listenAddr, option == kReusePort)),
    threadPool_(new EventLoopThreadPool(loop)),
    connectionCallback_(defaultConnectionCallback),
    messageCallback_(defaultMessageCallback),
    nextConnId_(1) {
  // Called when an connection is accpeted, @see Acceptor::HandleRead which is
  // called when a connection request is listened on 'listen fd'.
  acceptor_->setNewConnectionCallback(
      boost::bind(&TcpServer::newConnection, this, _1, _2));
}

TcpServer::~TcpServer() {
  loop_->assertInLoopThread();
  LOG_TRACE << "TcpServer::~TcpServer [" << name_ << "] deing";

  for (ConnectionMap::iterator iter = connections_.begin();
      iter != connections_.end(); ++iter) {
    TcpConnectionPtr conn = iter->second;
    iter->second.reset();
    conn->getLoop()->runInLoop(
      boost::bind(&TcpConnection::connectDestroyed, conn));
    conn.reset();
  }
}

void TcpServer::setThreadNum(int threads) {
  assert(0 <= threads);
  threadPool_->setThreadNum(threads);
}

void TcpServer::start() {
  if (started_.getAndSet(1) == 0) {
    threadPool_->start(threadInitCallback_);

    assert(!acceptor_->listenning());

    // Start listening and then accepting connections right now.
    loop_->runInLoop(
        boost::bind(&Acceptor::listen, get_pointer(acceptor_)));
  }
}

// Called when an connection is established, @see Acceptor::handleRead
void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr) {
  loop_->assertInLoopThread();

  // This new connection is assign to a event_loop thread in a round-robin way.
  EventLoop* ioLoop = threadPool_->getNextLoop();
  char buf[32];
  snprintf(buf, sizeof buf, ":%s#%d", hostport_.c_str(), nextConnId_);
  ++nextConnId_;
  string connName = name_ + buf;

  LOG_INFO << "TcpServer::newConnection [" << name_
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
  conn->setConnectionCallback(connectionCallback_);
  conn->setMessageCallback(messageCallback_);
  conn->setWriteCompleteCallback(writeCompleteCallback_);
  conn->setCloseCallback(
      boost::bind(&TcpServer::removeConnection, this, _1)); // FIXME: unsafe

  // Run TcpConnection::connectEstablished immediately.
  ioLoop->runInLoop(boost::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn) {
  // FIXME: unsafe
  loop_->runInLoop(boost::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn) {
  loop_->assertInLoopThread();
  LOG_INFO << "TcpServer::removeConnectionInLoop [" << name_
           << "] - connection " << conn->name();
  size_t n = connections_.erase(conn->name());
  (void)n;
  assert(n == 1);
  EventLoop* ioLoop = conn->getLoop();
  ioLoop->queueInLoop(
      boost::bind(&TcpConnection::connectDestroyed, conn));
}

}  // namespace cobra
