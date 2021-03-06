#ifndef COBRA_TCPCONNECTION_H_
#define COBRA_TCPCONNECTION_H_

#include <boost/any.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include "base/basic_types.h"
#include "base/macros.h"
#include "base/string_piece.h"
#include "base/Types.h"
#include "cobra/callbacks.h"
#include "cobra/buffer.h"
#include "cobra/endpoint.h"

namespace cobra {

class Channel;
class Worker;
class Socket;

// TCP connection, for both client and server usage.
class TcpConnection : public boost::enable_shared_from_this<TcpConnection> {
 public:
  TcpConnection(Worker* loop,
                const string& name,
                int sockfd,
                const Endpoint& localAddr,
                const Endpoint& peerAddr);
  ~TcpConnection();

  Worker* getLoop() const { return loop_; }
  const string& name() const { return name_; }
  const Endpoint& localAddress() { return localAddr_; }
  const Endpoint& peerAddress() { return peerAddr_; }
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
  // These callback functions are set by user, @see Server::SetConnectionCallBack
  inline void SetConnectionCb(const ConnectionCb& cb) {
    connectionCb_ = cb;
  }

  // @see TcpServer::SetMessageCallBack
  inline void SetMessageCb(const MessageCb& cb) {
    messageCb_ = cb;
  }

  // @see TcpServer::SetWriteCompleteCallBack
  inline void SetWriteCompleteCb(const WriteCompleteCb& cb) { 
    writeCompleteCb_ = cb;
  }
  /////////////////////////////////// end //////////////////////////////////////


  void setHighWaterMarkCb(const HighWaterMarkCb& cb,
                                size_t highWaterMark) {
    highWaterMarkCb_ = cb;
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
  void SetCloseCb(const CloseCb& cb) { closeCb_ = cb; }

  // Called when TcpServer accepts a new connection
  void ConnectionEstablished();   // should be called only once
  // Called when TcpServer has removed me from its map
  void ConnectionDestroyed();  // should be called only once

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

  Worker* loop_;
  string name_;
  StateE state_;  // FIXME: use atomic variable
  int32 conn_fd_;
  boost::scoped_ptr<Channel> channel_;
  Endpoint localAddr_;
  Endpoint peerAddr_;
  ConnectionCb connectionCb_;
  MessageCb messageCb_;
  WriteCompleteCb writeCompleteCb_;
  HighWaterMarkCb highWaterMarkCb_;
  CloseCb closeCb_;
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

#endif  // COBRA_TCPCONNECTION_H_
