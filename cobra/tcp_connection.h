#ifndef COBRA_NET_TCPCONNECTION_H
#define COBRA_NET_TCPCONNECTION_H

#include <boost/any.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include "base/macros.h"
#include "base/Mutex.h"
#include "base/StringPiece.h"
#include "base/Types.h"
#include "cobra/callbacks.h"
#include "cobra/buffer.h"
#include "cobra/inet_address.h"

namespace cobra {

class Channel;
class EventLoop;
class Socket;

// TCP connection, for both client and server usage.
class TcpConnection : public boost::enable_shared_from_this<TcpConnection> {
 public:
  TcpConnection(EventLoop* loop,
                const string& name,
                int sockfd,
                const InetAddress& localAddr,
                const InetAddress& peerAddr);
  ~TcpConnection();

  EventLoop* getLoop() const { return loop_; }
  const string& name() const { return name_; }
  const InetAddress& localAddress() { return localAddr_; }
  const InetAddress& peerAddress() { return peerAddr_; }
  bool connected() const { return state_ == kConnected; }

  void send(const void* message, size_t len);
  void send(const StringPiece& message);
  void send(Buffer* message);  // this one will swap data
  void shutdown(); // NOT thread safe, no simultaneous calling
  void setTcpNoDelay(bool on);

  void setContext(const boost::any& context) {
    context_ = context;
  }

  const boost::any& getContext() const {
    return context_;
  }

  boost::any* getMutableContext() {
    return &context_;
  }

  //////////////////////////////////// begin //////////////////////////////
  // These callback functions are set by user, @see TcpServer::SetConnectionCallBack
  void setConnectionCallback(const ConnectionCallback& cb)
  { connectionCallback_ = cb; }

  // @see TcpServer::SetMessageCallBack
  void setMessageCallback(const MessageCallback& cb)
  { messageCallback_ = cb; }

  // @see TcpServer::SetWriteCompleteCallBack
  void setWriteCompleteCallback(const WriteCompleteCallback& cb)
  { writeCompleteCallback_ = cb; }
  /////////////////////////////////// end //////////////////////////////////////


  void setHighWaterMarkCallback(const HighWaterMarkCallback& cb,
                                size_t highWaterMark) {
    highWaterMarkCallback_ = cb;
    highWaterMark_ = highWaterMark;
  }

  /// Advanced interface
  Buffer* inputBuffer() {
    return &inputBuffer_;
  }

  Buffer* outputBuffer() {
    return &outputBuffer_;
  }

  // Internal use only.
  void setCloseCallback(const CloseCallback& cb)
  { closeCallback_ = cb; }

  // Called when TcpServer accepts a new connection
  void connectEstablished();   // should be called only once
  // Called when TcpServer has removed me from its map
  void connectDestroyed();  // should be called only once

 private:
  enum StateE { kDisconnected, kConnecting, kConnected, kDisconnecting };
  void handleRead(Timestamp receiveTime);
  void handleWrite();
  void handleClose();
  void handleError();
  void sendInLoop(const StringPiece& message);
  void sendInLoop(const void* message, size_t len);
  void shutdownInLoop();
  void setState(StateE s) { state_ = s; }

  EventLoop* loop_;
  string name_;
  StateE state_;  // FIXME: use atomic variable
  // we don't expose those classes to client.
  boost::scoped_ptr<Socket> socket_;
  boost::scoped_ptr<Channel> channel_;
  InetAddress localAddr_;
  InetAddress peerAddr_;
  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  WriteCompleteCallback writeCompleteCallback_;
  HighWaterMarkCallback highWaterMarkCallback_;
  CloseCallback closeCallback_;
  size_t highWaterMark_;
  Buffer inputBuffer_;
  Buffer outputBuffer_; // FIXME: use list<Buffer> as output buffer.
  boost::any context_;
  // FIXME: creationTime_, lastReceiveTime_
  //        bytesReceived_, bytesSent_

  DISABLE_COPY_AND_ASSIGN(TcpConnection);
};

typedef boost::shared_ptr<TcpConnection> TcpConnectionPtr;

}  // namespace cobra

#endif  // COBRA_NET_TCPCONNECTION_H
