#ifndef COBRA_ACCEPTOR_H_
#define COBRA_ACCEPTOR_H_

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

#include "base/macros.h"
#include "cobra/channel.h"
#include "cobra/socket.h"

namespace cobra {

class Worker;
class Endpoint;

// Acceptor of incoming TCP connections.
class Acceptor {
 public:
  typedef boost::function<void (int sockfd,
                                const Endpoint&)> NewConnectionCb;

  Acceptor(Worker* loop, const Endpoint& listenAddr);
  ~Acceptor();

  // Called when a new connecion comes
  void setNewConnectionCb(const NewConnectionCb& cb) {
    newConnectionCb_ = cb;
  }

  bool listenning() const { return listenning_; }
  void listen();

 private:
  void handleRead();

  Worker* loop_;
  Socket acceptSocket_;
  Channel acceptChannel_;
  NewConnectionCb newConnectionCb_;
  bool listenning_;
  int idleFd_;

  DISABLE_COPY_AND_ASSIGN(Acceptor);
};

}  // namespace cobra

#endif  // COBRA_ACCEPTOR_H_
