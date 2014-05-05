#ifndef COBRA_BASE_THREADPOOL_H_
#define COBRA_BASE_THREADPOOL_H_

#include <cobra/base/Condition.h>
#include <cobra/base/Mutex.h>
#include <cobra/base/Thread.h>
#include <cobra/base/Types.h>

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include <deque>

namespace cobra
{

class ThreadPool : boost::noncopyable
{
 public:
  typedef boost::function<void ()> Task;

  explicit ThreadPool(const string& name = string());
  ~ThreadPool();

  // Must be called before start().
  void setMaxQueueSize(int maxSize) { maxQueueSize_ = maxSize; }

  void start(int numThreads);
  void stop();

  // Could block if maxQueueSize > 0
  void run(const Task& f);
#ifdef __GXX_EXPERIMENTAL_CXX0X__
  void run(Task&& f);
#endif

 private:
  bool isFull() const;
  void runInThread();
  Task take();

  MutexLock mutex_;
  Condition notEmpty_;
  Condition notFull_;
  string name_;
  boost::ptr_vector<cobra::Thread> threads_;
  std::deque<Task> queue_;
  size_t maxQueueSize_;
  bool running_;
};

}

#endif
