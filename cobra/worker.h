// Author: Jianbo Zhu
//
// The worker, running a loop to handle events.
// Each worker belongs to a thread, and this thread can only
// own one worker which is actually an event loop.

#ifndef COBRA_WORKER_H_
#define COBRA_WORKER_H_

#include <vector>

#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>

#include "base/macros.h"
#include "base/Mutex.h"
#include "base/Thread.h"
#include "base/timestamp.h"
#include "cobra/callbacks.h"
#include "cobra/timer_id.h"

namespace cobra {

class Channel;
class Poller;
class TimerQueue;

// The reactor.
// Realized as one reactor per thread.
class Worker {
 public:
  // User callbacks.
  typedef boost::function<void()> Functor;

  Worker();
  ~Worker();

  // Loops forever.
  //
  // Must be called in the same thread as creation of the object.
  void run();

  // Quits the event loop.
  //
  // This is not 100% thread safe, if you call through a raw pointer,
  // better to call through shared_ptr<Worker> for 100% safety.
  void quit();

  // Time when poll returns, usually means data arrivial.
  inline Timestamp pollReturnTime() const {
    return pollReturnTime_;
  }

  // Runs callback immediately in the loop thread.
  // It wakes up the loop, and run the cb.
  // If in the same loop thread, cb is run within the function.
  // Safe to call from other threads.
  void runInLoop(const Functor& cb);

  // Queues callback in the loop thread.
  // Runs after finish polling.
  // Safe to call from other threads.
  void queueInLoop(const Functor& cb);

  ////////////////////// begin /////////////////////////////////
  // timers

  // Runs callback at 'time'.
  // Safe to call from other threads.
  TimerId runAt(const Timestamp& time, const TimerCb& cb);

  // Runs callback after @c delay seconds.
  // Safe to call from other threads.
  TimerId runAfter(double delay, const TimerCb& cb);

  // Runs callback every @c interval seconds.
  // Safe to call from other threads.
  TimerId runEvery(double interval, const TimerCb& cb);

  // Cancels the timer.
  // Safe to call from other threads.
  void cancel(TimerId timerId);
  ///////////////////////// end ///////////////////////////////

  // Only for internal usage
  void wakeup();

  // Update the monitoring events(read/write/err ect) of an fd(the socket)
  // wrapped in a channel or adding a new fd to the system call 'poll'
  // to monitor.
  // @see PollPoller::updateChannel().
  void updateChannel(Channel* channel);

  // Remove a fd, which is monotoring by the 'poll'.
  void removeChannel(Channel* channel);

  // pid_t threadId() const { return threadId_; }
  void assertInLoopThread() {
    if (!isInLoopThread()) {
      abortNotInLoopThread();
    }
  }

  inline bool isInLoopThread() const {
    return threadId_ == CurrentThread::tid();
  }

  // bool callingPendingFunctors() const { return callingPendingFunctors_; }

  // Is we handling the event(read/write/err etc) now,
  // @see 'channel-->handleEvents()'
  inline bool eventHandling() const {
    return eventHandling_;
  }

  static Worker* getWorkerOfCurrentThread();

 private:
  void abortNotInLoopThread();

  bool looping_; /* atomic */
  bool quit_; /* atomic and shared between threads, okay on x86, I guess. */
  const pid_t threadId_;
  Timestamp pollReturnTime_;
  boost::scoped_ptr<TimerQueue> timerQueue_;
  boost::scoped_ptr<Poller> poller_;

  // The eventfd wait/inotify mechanism.
  int wakeupFd_;
  // unlike in TimerQueue, which is an internal class,
  // we don't expose Channel to client.
  boost::scoped_ptr<Channel> wakeupChannel_;

  // The registered callback function for 'eventfd'.
  void handleRead();  // waked up

  bool eventHandling_; /* atomic */
  typedef std::vector<Channel*> ChannelList;
  ChannelList activeChannels_;
  Channel* currentActiveChannel_;

  MutexLock mutex_;
  bool callingPendingFunctors_; /* atomic */
  std::vector<Functor> pendingFunctors_; // @GuardedBy mutex_
  // Execute the pending functions in the pendingFunctors_.
  void doPendingFunctors();

  DISABLE_COPY_AND_ASSIGN(Worker);
};

}  // namespace cobra

#endif  // COBRA_WORKER_H_
