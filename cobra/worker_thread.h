#ifndef COBRA_WORKERTHREAD_H_
#define COBRA_WORKERTHREAD_H_

#include "base/Condition.h"
#include "base/macros.h"
#include "base/Mutex.h"
#include "base/Thread.h"

namespace cobra {

class Worker;

class WorkerThread {
 public:
  typedef boost::function<void(Worker*)> ThreadInitCb;

  WorkerThread(const ThreadInitCb& cb = ThreadInitCb());
  ~WorkerThread();

  Worker* startLoop();

 private:
  void threadFunc();

  Worker* worker_;
  bool exiting_;
  Thread thread_;
  MutexLock mutex_;
  Condition cond_;
  ThreadInitCb callback_;

  DISABLE_COPY_AND_ASSIGN(WorkerThread);
};

}  // namespace cobra

#endif  // COBRA_WORKERTHREAD_H_
