#include "cobra/tcp_client.h"

#include <cstdio>

#include <boost/bind.hpp>

#include "base/Logging.h"
#include "cobra/connector.h"
#include "cobra/worker.h"
#include "cobra/socket_wrapper.h"

namespace cobra {

namespace detail {

void removeConnection(Worker* loop, const TcpConnectionPtr& conn) {
  loop->queueInLoop(boost::bind(&TcpConnection::connectDestroyed, conn));
}

void removeConnector(const ConnectorPtr& connector) {
  // TODO(zhu):
}

}  // namespace detail

TcpClient::TcpClient(Worker* loop,
                     const Endpoint& serverAddr,
                     const string& name)
  : loop_(CHECK_NOTNULL(loop)),
    connector_(new Connector(loop, serverAddr)),
    name_(name),
    connectionCb_(defaultConnectionCb),
    messageCb_(defaultMessageCb),
    retry_(false),
    connect_(true),
    nextConnId_(1) {
  // Called when a new connection is established.
  connector_->setNewConnectionCb(
      boost::bind(&TcpClient::newConnection, this, _1));

  // FIXME setConnectFailedCb
  LOG_INFO << "TcpClient::TcpClient[" << name_
           << "] - connector " << get_pointer(connector_);
}

TcpClient::~TcpClient() {
  LOG_INFO << "TcpClient::~TcpClient[" << name_
           << "] - connector " << get_pointer(connector_);
  TcpConnectionPtr conn;
  {
    MutexLockGuard lock(mutex_);
    conn = connection_;
  }
  if (conn) {
    // FIXME: not 100% safe, if we are in different thread
    CloseCb cb = boost::bind(&detail::removeConnection, loop_, _1);
    loop_->runInLoop(
        boost::bind(&TcpConnection::setCloseCb, conn, cb));
  } else {
    connector_->stop();
    // FIXME: HACK
    loop_->runAfter(1, boost::bind(&detail::removeConnector, connector_));
  }
}

void TcpClient::connect() {
  // FIXME: check state
  LOG_INFO << "TcpClient::connect[" << name_ << "] - connecting to "
           << connector_->serverAddress().toIpPort();
  connect_ = true;
  connector_->start();
}

void TcpClient::disconnect() {
  connect_ = false;

  {
    MutexLockGuard lock(mutex_);
    if (connection_) {
      connection_->shutdown();
    }
  }
}

void TcpClient::stop() {
  connect_ = false;
  connector_->stop();
}

// Called after an connection is established.
void TcpClient::newConnection(int sockfd) {
  loop_->assertInLoopThread();
  Endpoint peerAddr(internal::getPeerAddr(sockfd));
  char buf[32];
  snprintf(buf, sizeof buf, ":%s#%d", peerAddr.toIpPort().c_str(), nextConnId_);
  ++nextConnId_;
  string connName = name_ + buf;

  Endpoint localAddr(internal::getLocalAddr(sockfd));
  // FIXME poll with zero timeout to double confirm the new connection
  // FIXME use make_shared if necessary
  TcpConnectionPtr conn(new TcpConnection(loop_,
                                          connName,
                                          sockfd,
                                          localAddr,
                                          peerAddr));

  conn->setConnectionCb(connectionCb_);
  conn->setMessageCb(messageCb_);
  conn->setWriteCompleteCb(writeCompleteCb_);
  conn->setCloseCb(
      boost::bind(&TcpClient::removeConnection, this, _1)); // FIXME: unsafe
  {
    MutexLockGuard lock(mutex_);
    connection_ = conn;
  }

  conn->connectEstablished();
}

void TcpClient::removeConnection(const TcpConnectionPtr& conn) {
  loop_->assertInLoopThread();
  assert(loop_ == conn->getLoop());

  {
    MutexLockGuard lock(mutex_);
    assert(connection_ == conn);
    connection_.reset();
  }

  loop_->queueInLoop(boost::bind(&TcpConnection::connectDestroyed, conn));
  if (retry_ && connect_)
  {
    LOG_INFO << "TcpClient::connect[" << name_ << "] - Reconnecting to "
             << connector_->serverAddress().toIpPort();
    connector_->restart();
  }
}

}  // namespace cobra
