#ifndef COBRA_TCPSERVER_H_
#define COBRA_TCPSERVER_H_

#include <map>

#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>

#include "base/macros.h"
#include "base/Atomic.h"
#include "base/Types.h"
#include "cobra/tcp_connection.h"

namespace cobra {

class Acceptor;
class Worker;
class WorkerThreadPool;

class TcpServer {
 public:
  typedef boost::function<void(Worker*)> ThreadInitCb;
  enum Option {
    kNoReusePort,
    kReusePort,
  };

  TcpServer(Worker* loop,
            const InetAddress& listenAddr, // ip and port
            const string& nameArg,
            Option option = kNoReusePort);
  ~TcpServer();

  inline const string& hostport() const {
    return hostport_;
  }

  inline const string& name() const {
    return name_;
  }

  inline Worker* getLoop() const {
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
  void setThreadNum(int numThreads);
  inline void setThreadInitCb(const ThreadInitCb& cb) {
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
  void newConnection(int sockfd, const InetAddress& peerAddr);

  // Thread safe.
  void removeConnection(const TcpConnectionPtr& conn);

  // Not thread safe, but in loop
  void removeConnectionInLoop(const TcpConnectionPtr& conn);

  AtomicInt32 started_;
  Worker* loop_;  // the acceptor loop
  const string hostport_;
  const string name_;

  boost::scoped_ptr<Acceptor> acceptor_;
  boost::scoped_ptr<WorkerThreadPool> threadPool_;

  // The callbacks
  ThreadInitCb threadInitCb_;
  ConnectionCb connectionCb_;
  MessageCb messageCb_;
  WriteCompleteCb writeCompleteCb_;

  // The established connections
  typedef std::map<string, TcpConnectionPtr> ConnectionMap;
  ConnectionMap connections_;
  // Always in loop thread
  int nextConnId_;

  DISABLE_COPY_AND_ASSIGN(TcpServer);
};

}  // namespace cobra

#endif  // COBRA_TCPSERVER_H_
