#ifndef COBRA_NET_TCPCLIENT_H
#define COBRA_NET_TCPCLIENT_H

#include "cobra/base/macros.h"
#include "cobra/base/Mutex.h"
#include "cobra/net/tcp_connection.h"

namespace cobra {
namespace net {

class Connector;
typedef boost::shared_ptr<Connector> ConnectorPtr;

class TcpClient : boost::noncopyable {
 public:
  TcpClient(EventLoop* loop,
            const InetAddress& serverAddr,
            const string& name);
  ~TcpClient();

  void connect();
  void disconnect();
  void stop();

  TcpConnectionPtr connection() const {
    MutexLockGuard lock(mutex_);
    return connection_;
  }

  EventLoop* getLoop() const { return loop_; }
  bool retry() const;
  void enableRetry() { retry_ = true; }

  // Set connection callback.
  // Not thread safe.
  void setConnectionCallback(const ConnectionCallback& cb)
  { connectionCallback_ = cb; }

  // Set message callback.
  // Not thread safe.
  void setMessageCallback(const MessageCallback& cb)
  { messageCallback_ = cb; }

  // Set write complete callback.
  // Not thread safe.
  void setWriteCompleteCallback(const WriteCompleteCallback& cb)
  { writeCompleteCallback_ = cb; }

 private:
  // Not thread safe, but in loop
  void newConnection(int sockfd);
  // Not thread safe, but in loop
  void removeConnection(const TcpConnectionPtr& conn);

  EventLoop* loop_;
  ConnectorPtr connector_; // avoid revealing Connector
  const string name_;

  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  WriteCompleteCallback writeCompleteCallback_;

  bool retry_;   // atmoic
  bool connect_; // atomic
  // always in loop thread
  int nextConnId_;

  mutable MutexLock mutex_;
  TcpConnectionPtr connection_; // @GuardedBy mutex_
};

}  // namespace net
}  // namespace cobra

#endif  // COBRA_NET_TCPCLIENT_H
