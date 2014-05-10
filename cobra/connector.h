#ifndef COBRA_CONNECTOR_H_
#define COBRA_CONNECTOR_H_

#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>

#include "base/macros.h"
#include "cobra/endpoint.h"

namespace cobra {

class Channel;
class Worker;

class Connector : public boost::enable_shared_from_this<Connector> {
 public:
  typedef boost::function<void (int sockfd)> NewConnectionCb;

  Connector(Worker* loop, const Endpoint& serverAddr);
  ~Connector();

  void setNewConnectionCb(const NewConnectionCb& cb)
  { new_conn_cb_ = cb; }

  void start();  // can be called in any thread
  void restart();  // must be called in loop thread
  void stop();  // can be called in any thread

  const Endpoint& serverAddress() const { return serverAddr_; }

 private:
  enum States { kDisconnected, kConnecting, kConnected };
  States state_;  // FIXME: use atomic variable

  // 30 seconds
  static const int kMaxRetryDelayMs = 30000;
  static const int kInitRetryDelayMs = 500;

  void setState(States s) { state_ = s; }
  void startInLoop();
  void stopInLoop();
  void connect();
  void connecting(int sockfd);
  void handleWrite();
  void handleError();
  void retry(int sockfd);
  int removeAndResetChannel();
  void resetChannel();

  Worker* loop_;
  Endpoint serverAddr_;
  bool connect_; // atomic
  boost::scoped_ptr<Channel> channel_;
  NewConnectionCb new_conn_cb_;
  int retryDelayMs_;
};

}  // namespace cobra

#endif  // COBRA_CONNECTOR_H_
