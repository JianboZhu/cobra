#ifndef COBRA_NET_TIMERQUEUE_H
#define COBRA_NET_TIMERQUEUE_H

#include <set>
#include <vector>

#include "cobra/base/macros.h"
#include "cobra/base/timestamp.h"
#include "cobra/net/callbacks.h"
#include "cobra/net/channel.h"

namespace cobra {
namespace net {

class EventLoop;
class Timer;
class TimerId;

// A best efforts timer queue.
class TimerQueue {
 public:
  TimerQueue(EventLoop* loop);
  ~TimerQueue();

  // Schedules the callback to be run at given time,
  // repeats if @c interval > 0.0.
  //
  // Must be thread safe. Usually be called from other threads.
  TimerId addTimer(const TimerCallback& cb,
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

  EventLoop* loop_;
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

}  // namespace net
}  // namespace cobra

#endif  // COBRA_NET_TIMERQUEUE_H
