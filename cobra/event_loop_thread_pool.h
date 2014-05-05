#ifndef COBRA_NET_EVENTLOOPTHREADPOOL_H_
#define COBRA_NET_EVENTLOOPTHREADPOOL_H_

#include <vector>

#include <boost/function.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include "base/macros.h"
#include "base/Condition.h"
#include "base/Mutex.h"

namespace cobra {

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool {
 public:
  typedef boost::function<void(EventLoop*)> ThreadInitCallback;

  EventLoopThreadPool(EventLoop* baseLoop);
  ~EventLoopThreadPool();

  void setThreadNum(int numThreads) { numThreads_ = numThreads; }
  void start(const ThreadInitCallback& cb = ThreadInitCallback());
  EventLoop* getNextLoop();

 private:
  EventLoop* baseLoop_;
  bool started_;
  int numThreads_;
  int next_;
  boost::ptr_vector<EventLoopThread> threads_;
  std::vector<EventLoop*> loops_;

  DISABLE_COPY_AND_ASSIGN(EventLoopThreadPool);
};

}  // namespace cobra

#endif  // COBRA_NET_EVENTLOOPTHREADPOOL_H_
