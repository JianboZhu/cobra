#ifndef COBRA_NET_ACCEPTOR_H_
#define COBRA_NET_ACCEPTOR_H_

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

#include "cobra/base/macros.h"
#include "cobra/net/channel.h"
#include "cobra/net/socket.h"

namespace cobra {
namespace net {

class EventLoop;
class InetAddress;

// Acceptor of incoming TCP connections.
class Acceptor {
 public:
  typedef boost::function<void (int sockfd,
                                const InetAddress&)> NewConnectionCallback;

  Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport);
  ~Acceptor();

  void setNewConnectionCallback(const NewConnectionCallback& cb) {
    newConnectionCallback_ = cb;
  }

  bool listenning() const { return listenning_; }
  void listen();

 private:
  void handleRead();

  EventLoop* loop_;
  Socket acceptSocket_;
  Channel acceptChannel_;
  NewConnectionCallback newConnectionCallback_;
  bool listenning_;
  int idleFd_;

  DISABLE_COPY_AND_ASSIGN(Acceptor);
};

}  // namespace net
}  // namespace cobra

#endif  // COBRA_NET_ACCEPTOR_H_
