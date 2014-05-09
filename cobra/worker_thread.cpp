#include "cobra/worker_thread.h"

#include <boost/bind.hpp>

#include "cobra/worker.h"

namespace cobra {

WorkerThread::WorkerThread(const ThreadInitCb& cb)
  : worker_(NULL),
    exiting_(false),
    thread_(boost::bind(&WorkerThread::threadFunc, this)),
    mutex_(),
    cond_(mutex_),
    callback_(cb) {
}

WorkerThread::~WorkerThread() {
  exiting_ = true;
  // not 100% race-free, eg. threadFunc could be running callback_.
  if (worker_ != NULL) {
    // still a tiny chance to call deed object, if threadFunc exits just now.
    // but when WorkerThread des, usually programming is exiting anyway.
    worker_->quit();
    thread_.join();
  }
}

Worker* WorkerThread::startLoop() {
  assert(!thread_.started());
  thread_.start();

  {
    MutexLockGuard lock(mutex_);
    while (worker_ == NULL) {
      cond_.wait();
    }
  }

  return worker_;
}

// Start an event loop in every thread from the thread pool.
void WorkerThread::threadFunc() {
  Worker worker;

  if (callback_) {
    callback_(&worker);
  }

  {
    MutexLockGuard lock(mutex_);
    worker_ = &worker;
    cond_.notify();
  }

  worker.run();

  worker_ = NULL;
}

}  // namespace cobra
