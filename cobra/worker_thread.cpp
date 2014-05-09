// Author: Jianbo Zhu
//

#include "cobra/worker_thread.h"

#include <boost/bind.hpp>

#include "cobra/worker.h"

namespace cobra {

WorkerThread::WorkerThread(const ThreadInitCb& cb)
  : init_cb_(cb),
    worker_(NULL),
    exiting_(false) {
}

WorkerThread::~WorkerThread() {
  exiting_ = true;
  // not 100% race-free, eg. threadFunc could be running init_cb_.
  if (worker_) {
    // still a tiny chance to call deed object, if threadFunc exits just now.
    // but when WorkerThread des, usually programming is exiting anyway.
    worker_->Quit();
    thread_.join();
  }
}

Worker* WorkerThread::StartLoop() {
  assert(!thread_.started());
  thread_ = boost::thread(boost::bind(&WorkerThread::ThreadFunc, this));

  {
    boost::mutex::scoped_lock lock(mutex_);
    while (!worker_) {
      cond_.wait(lock);
    }
  }

  return worker_;
}

// Start an event loop in every thread from the thread pool.
void WorkerThread::ThreadFunc() {
  Worker worker;

  if (init_cb_) {
    init_cb_(&worker);
  }

  {
    boost::mutex::scoped_lock lock(mutex_);
    worker_ = &worker;
    cond_.notify_one();
  }

  worker.Loop();

  worker_ = NULL;
}

}  // namespace cobra
