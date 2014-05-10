#ifndef COBRA_TIMERQUEUE_H_
#define COBRA_TIMERQUEUE_H_

#include <set>
#include <vector>

#include "base/macros.h"
#include "base/timestamp.h"
#include "cobra/callbacks.h"
#include "cobra/channel.h"

namespace cobra {

class Worker;
class Timer;
class TimerId;

// A best efforts timer queue.
class TimerQueue {
 public:
  TimerQueue(Worker* loop);
  ~TimerQueue();

  // Schedules the callback to be run at given time,
  // repeats if @c interval > 0.0.
  //
  // Must be thread safe. Usually be called from other threads.
  TimerId addTimer(const TimerCb& cb,
                   Timestamp when,
                   double interval);

  void cancel(TimerId timerId);

 private:

  // FIXME use unique_ptr<Timer> instead of raw pointers.
  typedef std::pair<Timestamp, Timer*> Entry;
  typedef std::set<Entry> TimerList;
  typedef std::pair<Timer*, int64_t> ActiveTimer;
  typedef std::set<ActiveTimer> ActiveTimerSet;

  void addTimerInLoop(Timer* timer);
  void cancelInLoop(TimerId timerId);

  // called when timerfd alarms
  void handleRead();

  // move out all expired timers
  std::vector<Entry> getExpired(Timestamp now);
  void reset(const std::vector<Entry>& expired, Timestamp now);

  bool insert(Timer* timer);

  Worker* loop_;
  const int timerfd_;
  Channel timerfdChannel_;
  // Timer list sorted by expiration
  TimerList timers_;

  // For cancel()
  ActiveTimerSet activeTimers_;
  bool callingExpiredTimers_; /* atomic */
  ActiveTimerSet cancelingTimers_;

  DISABLE_COPY_AND_ASSIGN(TimerQueue);
};

}  // namespace cobra

#endif  // COBRA_TIMERQUEUE_H_
