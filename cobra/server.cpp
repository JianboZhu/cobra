#include "cobra/server.h"

#include <cstdio>

#include <boost/bind.hpp>

#include "base/Logging.h"
#include "cobra/acceptor.h"
#include "cobra/worker.h"
#include "cobra/worker_thread_pool.h"
#include "cobra/socket_wrapper.h"

namespace cobra {

Server::Server(Worker* loop,
               const Endpoint& listenAddr,
               const string& server_name)
  : loop_(CHECK_NOTNULL(loop)),
    hostport_(listenAddr.toIpPort()),
    name_(server_name),
    acceptor_(new Acceptor(loop, listenAddr)),
    thread_pool_(new WorkerThreadPool(loop)),
    connectionCb_(defaultConnectionCb),
    messageCb_(defaultMessageCb),
    nextConnId_(1) {
  // Called when an connection is accpeted, @see Acceptor::HandleRead which is
  // called when a connection request is listened on 'listen fd'.
  acceptor_->setNewConnectionCb(
      boost::bind(&Server::EstablishConnection, this, _1, _2));
}

Server::~Server() {
  loop_->assertInLoopThread();
  LOG_TRACE << "Server::~Server [" << name_ << "] dying";

  for (ConnectionMap::iterator iter = connections_.begin();
      iter != connections_.end(); ++iter) {
    TcpConnectionPtr conn = iter->second;
    iter->second.reset();
    conn->getLoop()->runInLoop(
      boost::bind(&TcpConnection::ConnectionDestroyed, conn));
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
        boost::bind(&Acceptor::Listen, get_pointer(acceptor_)));
  }
}

void Server::EstablishConnection(int32 conn_fd, const Endpoint& peer_address) {
  loop_->assertInLoopThread();

  // This new connection is assign to a event_loop thread in a round-robin way.
  Worker* ioLoop = thread_pool_->getNextLoop();
  char buf[32];
  snprintf(buf, sizeof buf, ":%s#%d", hostport_.c_str(), nextConnId_);
  ++nextConnId_;
  string connName = name_ + buf;

  LOG_INFO << "Server::EstablishConnection [" << name_
           << "] - new connection [" << connName
           << "] from " << peer_address.toIpPort();
  Endpoint local_address(internal::getLocalAddr(conn_fd));
  // FIXME poll with zero timeout to double confirm the new connection
  // FIXME use make_shared if necessary
  TcpConnectionPtr conn(new TcpConnection(ioLoop,
                                          connName,
                                          conn_fd,
                                          local_address,
                                          peer_address));

  // Records the new connection.
  connections_[connName] = conn;

  // Set callbacks for this connection.
  conn->setConnectionCb(connectionCb_);
  conn->setMessageCb(messageCb_);
  conn->setWriteCompleteCb(writeCompleteCb_);
  conn->setCloseCb(
      boost::bind(&Server::removeConnection, this, _1)); // FIXME: unsafe

  // Run TcpConnection::connectEstablished immediately.
  ioLoop->runInLoop(boost::bind(&TcpConnection::ConnectionEstablished, conn));
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
  Worker* ioLoop = conn->getLoop();
  ioLoop->queueInLoop(
      boost::bind(&TcpConnection::ConnectionDestroyed, conn));
}

}  // namespace cobra
