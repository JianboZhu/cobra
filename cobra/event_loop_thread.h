#ifndef COBRA_NET_EVENTLOOPTHREAD_H_
#define COBRA_NET_EVENTLOOPTHREAD_H_

#include "cobra/base/Condition.h"
#include "cobra/base/macros.h"
#include "cobra/base/Mutex.h"
#include "cobra/base/Thread.h"

namespace cobra {
namespace net {

class EventLoop;

class EventLoopThread {
 public:
  typedef boost::function<void(EventLoop*)> ThreadInitCallback;

  EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback());
  ~EventLoopThread();

  EventLoop* startLoop();

 private:
  void threadFunc();

  EventLoop* loop_;
  bool exiting_;
  Thread thread_;
  MutexLock mutex_;
  Condition cond_;
  ThreadInitCallback callback_;

  DISABLE_COPY_AND_ASSIGN(EventLoopThread);
};

}  // namespace net
}  // namespace cobra

#endif  // COBRA_NET_EVENTLOOPTHREAD_H_
