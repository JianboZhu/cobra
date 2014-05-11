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
#include "cobra/tcp_connection.h"

namespace cobra {

class Acceptor;
class Worker;
class WorkerThreadPool;

class Server {
 public:
  typedef boost::function<void(Worker*)> ThreadInitCb;

  Server(Worker* loop,
         const Endpoint& listen_address,
         const string& server_name = "server");
  virtual ~Server();

  inline Worker* GetWorker() const {
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
  inline void SetConnectionCb(const ConnectionCb& cb) {
    connectionCb_ = cb;
  }

  // Set message callback.
  // Not thread safe.
  //
  // Called when a message is readed into the input_buffer.
  inline void SetMessageCb(const MessageCb& cb) {
    messageCb_ = cb;
  }

  // Set write complete callback.
  // Not thread safe.
  //
  // Called when a message is writed complete.
  inline void SetWriteCompleteCb(const WriteCompleteCb& cb) {
    writeCompleteCb_ = cb;
  }

 private:
  // Not thread safe, but in loop
  //
  // Establish a connection.
  // It's used as a callback function.
  void EstablishConnection(int32 sockfd, const Endpoint& peerAddr);

  // Thread safe.
  void RemoveConnection(const TcpConnectionPtr& conn);

  // Not thread safe, but in loop
  void RemoveConnectionInLoop(const TcpConnectionPtr& conn);

  bool started_;
  Worker* loop_;  // the acceptor loop
  const string hostport_;
  const string name_;

  boost::scoped_ptr<Acceptor> acceptor_;
  boost::scoped_ptr<WorkerThreadPool> thread_pool_;

  // The callback functions
  ThreadInitCb threadInitCb_;
  ConnectionCb connectionCb_;
  MessageCb messageCb_;
  WriteCompleteCb writeCompleteCb_;

  // The established connections
  typedef std::map<string, TcpConnectionPtr> ConnectionMap;
  ConnectionMap connections_;

  DISABLE_COPY_AND_ASSIGN(Server);
};

}  // namespace cobra

#endif  // COBRA_SERVER_H_
