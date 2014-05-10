#ifndef COBRA_TIMERID_H_
#define COBRA_TIMERID_H_

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

#endif  // COBRA_TIMERID_H_
