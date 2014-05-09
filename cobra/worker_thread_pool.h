#ifndef COBRA_WORKERTHREADPOOL_H_
#define COBRA_WORKERTHREADPOOL_H_

#include <vector>

#include <boost/function.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include "base/macros.h"
#include "base/Condition.h"
#include "base/Mutex.h"

namespace cobra {

class Worker;
class WorkerThread;

class WorkerThreadPool {
 public:
  typedef boost::function<void(Worker*)> ThreadInitCb;

  WorkerThreadPool(Worker* baseLoop);
  ~WorkerThreadPool();

  void setThreadNum(int numThreads) { numThreads_ = numThreads; }
  void start(const ThreadInitCb& cb = ThreadInitCb());
  Worker* getNextLoop();

 private:
  Worker* baseLoop_;
  bool started_;
  int numThreads_;
  int next_;
  boost::ptr_vector<WorkerThread> threads_;
  std::vector<Worker*> loops_;

  DISABLE_COPY_AND_ASSIGN(WorkerThreadPool);
};

}  // namespace cobra

#endif  // COBRA_WORKERTHREADPOOL_H_
