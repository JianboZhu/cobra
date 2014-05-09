// Author: Jianbo Zhu
//
// The worker thread.
// One worker thread own and only own one worker (one event loop).

#ifndef COBRA_WORKERTHREAD_H_
#define COBRA_WORKERTHREAD_H_

#include <boost/function.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>

#include "base/macros.h"

namespace cobra {

class Worker;

class WorkerThread {
 public:
  typedef boost::function<void(Worker*)> ThreadInitCb;

  WorkerThread(const ThreadInitCb& cb = ThreadInitCb());
  ~WorkerThread();

  Worker* StartLoop();

 private:
  void ThreadFunc();
  ThreadInitCb init_cb_;

  Worker* worker_;

  boost::thread thread_;
  boost::mutex mutex_;
  boost::condition_variable cond_;

  bool exiting_;

  DISABLE_COPY_AND_ASSIGN(WorkerThread);
};

}  // namespace cobra

#endif  // COBRA_WORKERTHREAD_H_
