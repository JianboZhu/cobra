#include "cobra/worker.h"

#include "base/Logging.h"
#include "base/Mutex.h"
#include "cobra/channel.h"
#include "cobra/poller.h"
#include "cobra/socket_wrapper.h"
#include "cobra/timer_queue.h"

#include <boost/bind.hpp>

#include <signal.h>
#include <sys/eventfd.h>

namespace cobra {

namespace {
// The thread local var.
__thread Worker* t_loopInThisThread = 0;

const int kPollTimeMs = 10000;

// An event wait/notify mechanism by user-space applications, and by the kernel
// to notify user-space applications of events.
int createEventfd() {
  int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtfd < 0) {
    LOG_SYSERR << "Failed in eventfd";
    abort();
  }

  return evtfd;
}

// Ignore the 'sigpipe' signal which can abort your program.
struct IgnoreSigPipe {
  IgnoreSigPipe() {
    ::signal(SIGPIPE, SIG_IGN);

    LOG_TRACE << "Ignore SIGPIPE";
  }
} initObj;

}  // Anonymous namespace

Worker* Worker::getWorkerOfCurrentThread() {
  return t_loopInThisThread;
}

Worker::Worker()
  : looping_(false),
    quit_(false),
    threadId_(CurrentThread::tid()),
    timerQueue_(new TimerQueue(this)),
    poller_(Poller::newDefaultPoller(this)), // TODO(zhujianbo): Don't call this in ctor.
    wakeupFd_(createEventfd()),
    wakeupChannel_(new Channel(this, wakeupFd_)),
    eventHandling_(false),
    currentActiveChannel_(NULL),
    callingPendingFunctors_(false) {
  LOG_DEBUG << "Worker created " << this << " in thread " << threadId_;
  if (t_loopInThisThread) {
    LOG_FATAL << "Another Worker " << t_loopInThisThread
              << " exists in this thread " << threadId_;
  } else {
    t_loopInThisThread = this;
  }

  wakeupChannel_->setReadCb(
      boost::bind(&Worker::handleRead, this));

  // We are always reading the wakeupfd
  wakeupChannel_->enableReading();
}

Worker::~Worker() {
  LOG_DEBUG << "Worker " << this << " of thread " << threadId_
            << " des in thread " << CurrentThread::tid();
  ::close(wakeupFd_);
  t_loopInThisThread = NULL;
}

void Worker::run() {
  assert(!looping_);
  assertInLoopThread();
  looping_ = true;
  quit_ = false;  // FIXME: what if someone calls quit() before run() ?
  LOG_TRACE << "Worker " << this << " start looping";

  while (!quit_) {
    activeChannels_.clear();
    // Get available fds in current.
    pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);

    eventHandling_ = true;
    for (ChannelList::iterator iter = activeChannels_.begin();
        iter != activeChannels_.end(); ++iter) {
      currentActiveChannel_ = *iter;

      // Handle the read/write/err/close etc events.
      currentActiveChannel_->handleEvent(pollReturnTime_);
    }

    currentActiveChannel_ = NULL;
    eventHandling_ = false;

    doPendingFunctors();
  }

  LOG_TRACE << "Worker " << this << " stop looping";
  looping_ = false;
}

void Worker::quit() {
  quit_ = true;
  // There is a chance that run() just executes while(!quit_) and exists,
  // then Worker des, then we are accessing an invalid object.
  // Can be fixed using mutex_ in both places.
  if (!isInLoopThread()) {
    wakeup();
  }
}

void Worker::runInLoop(const Functor& cb) {
  if (isInLoopThread()) {
    cb();
  } else {
    queueInLoop(cb);
  }
}

void Worker::queueInLoop(const Functor& cb) {
  {
    MutexLockGuard lock(mutex_);
    pendingFunctors_.push_back(cb);
  }

  if (!isInLoopThread() || callingPendingFunctors_) {
    wakeup();
  }
}

TimerId Worker::runAt(const Timestamp& time, const TimerCb& cb) {
  return timerQueue_->addTimer(cb, time, 0.0);
}

TimerId Worker::runAfter(double delay, const TimerCb& cb) {
  Timestamp time(addTime(Timestamp::now(), delay));
  return runAt(time, cb);
}

TimerId Worker::runEvery(double interval, const TimerCb& cb) {
  Timestamp time(addTime(Timestamp::now(), interval));
  return timerQueue_->addTimer(cb, time, interval);
}

void Worker::cancel(TimerId timerId) {
  return timerQueue_->cancel(timerId);
}

void Worker::updateChannel(Channel* channel) {
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  poller_->updateChannel(channel);
}

void Worker::removeChannel(Channel* channel) {
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  if (eventHandling_) {
    assert(currentActiveChannel_ == channel ||
        std::find(activeChannels_.begin(), activeChannels_.end(), channel) == activeChannels_.end());
  }

  poller_->removeChannel(channel);
}

void Worker::abortNotInLoopThread() {
  LOG_FATAL << "Worker::abortNotInLoopThread - Worker " << this
            << " was created in threadId_ = " << threadId_
            << ", current thread id = " <<  CurrentThread::tid();
}

void Worker::wakeup() {
  uint64_t one = 1;
  ssize_t n = internal::write(wakeupFd_, &one, sizeof one);
  if (n != sizeof one) {
    LOG_ERROR << "Worker::wakeup() writes " << n << " bytes instead of 8";
  }
}

void Worker::handleRead() {
  uint64_t one = 1;
  ssize_t n = internal::read(wakeupFd_, &one, sizeof one);
  if (n != sizeof one) {
    LOG_ERROR << "Worker::handleRead() reads " << n << " bytes instead of 8";
  }
}

void Worker::doPendingFunctors() {
  std::vector<Functor> functors;
  callingPendingFunctors_ = true;

  {
    MutexLockGuard lock(mutex_);
    functors.swap(pendingFunctors_);
  }

  std::vector<Functor>::const_iterator iter = functors.begin();
  for (; iter != functors.end(); ++iter) {
    (*iter)();
  }

  callingPendingFunctors_ = false;
}

}  // namespace cobra
