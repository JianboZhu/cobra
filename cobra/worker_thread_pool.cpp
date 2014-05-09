#include "cobra/worker_thread_pool.h"

#include <boost/bind.hpp>

#include "cobra/worker.h"
#include "cobra/worker_thread.h"

namespace cobra {

WorkerThreadPool::WorkerThreadPool(Worker* baseLoop)
  : baseLoop_(baseLoop),
    started_(false),
    numThreads_(0),
    next_(0) {
}

WorkerThreadPool::~WorkerThreadPool() {
}

void WorkerThreadPool::start(const ThreadInitCb& cb) {
  assert(!started_);
  baseLoop_->assertInLoopThread();

  started_ = true;

  // Start event loop threads
  for (int i = 0; i < numThreads_; ++i) {
    WorkerThread* t = new WorkerThread(cb);
    threads_.push_back(t);
    loops_.push_back(t->StartLoop());
  }

  if (numThreads_ == 0 && cb) {
    cb(baseLoop_);
  }
}

Worker* WorkerThreadPool::getNextLoop() {
  baseLoop_->assertInLoopThread();
  Worker* loop = baseLoop_;

  if (!loops_.empty()) {
    // round-robin
    loop = loops_[next_];
    ++next_;
    if (implicit_cast<size_t>(next_) >= loops_.size()) {
      next_ = 0;
    }
  }

  return loop;
}

}  // namespace cobra
