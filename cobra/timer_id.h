#ifndef COBRA_NET_TIMERID_H
#define COBRA_NET_TIMERID_H

namespace cobra {

class Timer;

// An opaque identifier, for canceling Timer.
class TimerId {
 public:
  TimerId()
    : timer_(NULL),
      sequence_(0) {
  }

  TimerId(Timer* timer, int64_t seq)
    : timer_(timer),
      sequence_(seq) {
  }

  friend class TimerQueue;

 private:
  Timer* timer_;
  int64_t sequence_;
};

}  // namespace cobra

#endif  // COBRA_NET_TIMERID_H
