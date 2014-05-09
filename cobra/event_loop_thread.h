#ifndef COBRA_EVENTLOOPTHREAD_H_
#define COBRA_EVENTLOOPTHREAD_H_

#include "base/Condition.h"
#include "base/macros.h"
#include "base/Mutex.h"
#include "base/Thread.h"

namespace cobra {

class EventLoop;

class EventLoopThread {
 public:
  typedef boost::function<void(EventLoop*)> ThreadInitCb;

  EventLoopThread(const ThreadInitCb& cb = ThreadInitCb());
  ~EventLoopThread();

  EventLoop* startLoop();

 private:
  void threadFunc();

  EventLoop* loop_;
  bool exiting_;
  Thread thread_;
  MutexLock mutex_;
  Condition cond_;
  ThreadInitCb callback_;

  DISABLE_COPY_AND_ASSIGN(EventLoopThread);
};

}  // namespace cobra

#endif  // COBRA_EVENTLOOPTHREAD_H_
