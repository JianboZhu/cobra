// Author: Jianbo Zhu
//
// The acceptor.

#ifndef COBRA_ACCEPTOR_H_
#define COBRA_ACCEPTOR_H_

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

#include "base/basic_types.h"
#include "base/macros.h"
#include "cobra/channel.h"

namespace cobra {

class Worker;
class Endpoint;

// Acceptor of incoming TCP connections.
class Acceptor {
 public:
  typedef boost::function<void (int sockfd,
                                const Endpoint&)> NewConnectionCb;

  Acceptor(Worker* loop, const Endpoint& listen_address);
  ~Acceptor();

  // Called when a new connecion comes
  void SetNewConnectionCb(const NewConnectionCb& cb) {
    new_conn_cb_ = cb;
  }

  inline bool Listenning() const { return listenning_; }
  void Listen();

 private:
  void HandleRead();

  Worker* loop_;
  int32 listen_fd_;
  Channel accept_channel_;
  NewConnectionCb new_conn_cb_;
  bool listenning_;
  int32 idle_fd_;

  DISABLE_COPY_AND_ASSIGN(Acceptor);
};

}  // namespace cobra

#endif  // COBRA_ACCEPTOR_H_
