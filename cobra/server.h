// Author: Jianbo Zhu
//
// Interface for the server side.

#ifndef COBRA_SERVER_H_
#define COBRA_SERVER_H_

#include <map>

#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>

#include "base/basic_types.h"
#include "base/macros.h"
#include "base/Atomic.h"
#include "base/Types.h"
#include "cobra/tcp_connection.h"

namespace cobra {

class Acceptor;
class EventLoop;
class EventLoopThreadPool;

class Server {
 public:
  typedef boost::function<void(EventLoop*)> ThreadInitCb;

  Server(EventLoop* loop,
         const InetAddress& listenAddr,
         const string& nameArg);
  virtual ~Server();

  inline EventLoop* GetEventLoop() const {
    return loop_;
  }

  // Set the number of threads for handling input.
  //
  // Always accepts new connection in loop's thread.
  // Must be called before @c start
  // @param numThreads
  // - 0 means all I/O in loop's thread, no thread will created.
  //   this is the default value.
  // - 1 means all I/O in another thread.
  // - N means a thread pool with N threads, new connections
  //   are assigned on a round-robin basis.
  void SetThreadNum(uint32 numThreads);
  inline void SetThreadInitCb(const ThreadInitCb& cb) {
    threadInitCb_ = cb;
  }

  // Starts the server if it's not listenning.
  //
  // It's harmless to call it multiple times.
  // Thread safe.
  void start();

  // Set connection callback.
  // Not thread safe.
  //
  // Called when a connection is established.
  inline void setConnectionCb(const ConnectionCb& cb) {
    connectionCb_ = cb;
  }

  // Set message callback.
  // Not thread safe.
  //
  // Called when a message is readed into the input_buffer.
  inline void setMessageCb(const MessageCb& cb) {
    messageCb_ = cb;
  }

  // Set write complete callback.
  // Not thread safe.
  //
  // Called when a message is writed complete.
  inline void setWriteCompleteCb(const WriteCompleteCb& cb) {
    writeCompleteCb_ = cb;
  }

 private:
  // Not thread safe, but in loop
  //
  // It can be used as callback functions,
  // called when an connetion is established.
  void newConnection(int32 sockfd, const InetAddress& peerAddr);

  // Thread safe.
  void removeConnection(const TcpConnectionPtr& conn);

  // Not thread safe, but in loop
  void removeConnectionInLoop(const TcpConnectionPtr& conn);

  AtomicInt32 started_;
  EventLoop* loop_;  // the acceptor loop
  const string hostport_;
  const string name_;

  boost::scoped_ptr<Acceptor> acceptor_;
  boost::scoped_ptr<EventLoopThreadPool> thread_pool_;

  // The callback functions
  ThreadInitCb threadInitCb_;
  ConnectionCb connectionCb_;
  MessageCb messageCb_;
  WriteCompleteCb writeCompleteCb_;

  // The established connections
  typedef std::map<string, TcpConnectionPtr> ConnectionMap;
  ConnectionMap connections_;
  // Always in loop thread
  uint32 nextConnId_;

  DISABLE_COPY_AND_ASSIGN(Server);
};

}  // namespace cobra

#endif  // COBRA_SERVER_H_
