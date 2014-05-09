#include "cobra/event_loop_thread.h"

#include <boost/bind.hpp>

#include "cobra/event_loop.h"

namespace cobra {

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb)
  : loop_(NULL),
    exiting_(false),
    thread_(boost::bind(&EventLoopThread::threadFunc, this)),
    mutex_(),
    cond_(mutex_),
    callback_(cb) {
}

EventLoopThread::~EventLoopThread() {
  exiting_ = true;
  // not 100% race-free, eg. threadFunc could be running callback_.
  if (loop_ != NULL) {
    // still a tiny chance to call deed object, if threadFunc exits just now.
    // but when EventLoopThread des, usually programming is exiting anyway.
    loop_->quit();
    thread_.join();
  }
}

EventLoop* EventLoopThread::startLoop() {
  assert(!thread_.started());
  thread_.start();

  {
    MutexLockGuard lock(mutex_);
    while (loop_ == NULL) {
      cond_.wait();
    }
  }

  return loop_;
}

// Start an event loop in every thread from the thread pool.
void EventLoopThread::threadFunc() {
  EventLoop loop;

  if (callback_) {
    callback_(&loop);
  }

  {
    MutexLockGuard lock(mutex_);
    loop_ = &loop;
    cond_.notify();
  }

  loop.loop();

  loop_ = NULL;
}

}  // namespace cobra
