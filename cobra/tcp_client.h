#ifndef COBRA_TCPCLIENT_H
#define COBRA_TCPCLIENT_H

#include "base/macros.h"
#include "base/Mutex.h"
#include "cobra/tcp_connection.h"

namespace cobra {

class Connector;
typedef boost::shared_ptr<Connector> ConnectorPtr;

class TcpClient : boost::noncopyable {
 public:
  TcpClient(Worker* loop,
            const Endpoint& serverAddr,
            const string& name);
  ~TcpClient();

  void connect();
  void disconnect();
  void stop();

  TcpConnectionPtr connection() const {
    MutexLockGuard lock(mutex_);
    return connection_;
  }

  Worker* getLoop() const { return loop_; }
  bool retry() const;
  void enableRetry() { retry_ = true; }

  // Set connection callback.
  // Not thread safe.
  void setConnectionCb(const ConnectionCb& cb)
  { connectionCb_ = cb; }

  // Set message callback.
  // Not thread safe.
  void setMessageCb(const MessageCb& cb)
  { messageCb_ = cb; }

  // Set write complete callback.
  // Not thread safe.
  void setWriteCompleteCb(const WriteCompleteCb& cb)
  { writeCompleteCb_ = cb; }

 private:
  // Not thread safe, but in loop
  void newConnection(int sockfd);
  // Not thread safe, but in loop
  void removeConnection(const TcpConnectionPtr& conn);

  Worker* loop_;
  ConnectorPtr connector_; // avoid revealing Connector
  const string name_;

  ConnectionCb connectionCb_;
  MessageCb messageCb_;
  WriteCompleteCb writeCompleteCb_;

  bool retry_;   // atmoic
  bool connect_; // atomic
  // always in loop thread
  int nextConnId_;

  mutable MutexLock mutex_;
  TcpConnectionPtr connection_; // @GuardedBy mutex_
};

}  // namespace cobra

#endif  // COBRA_TCPCLIENT_H
