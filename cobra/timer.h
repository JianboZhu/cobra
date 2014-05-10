#ifndef COBRA_TIMER_H_
#define COBRA_TIMER_H_

#include "base/macros.h"
#include "base/Atomic.h"
#include "base/timestamp.h"
#include "cobra/callbacks.h"

namespace cobra {

class Timer {
 public:
  Timer(const TimerCb& cb, Timestamp when, double interval)
    : callback_(cb),
      expiration_(when),
      interval_(interval),
      repeat_(interval > 0.0),
      sequence_(s_numCreated_.incrementAndGet()) {
  }

  void run() const {
    callback_();
  }

  Timestamp expiration() const  { return expiration_; }
  bool repeat() const { return repeat_; }
  int64_t sequence() const { return sequence_; }

  void restart(Timestamp now);

  static int64_t numCreated() { return s_numCreated_.get(); }

 private:
  const TimerCb callback_;
  Timestamp expiration_;
  const double interval_;
  const bool repeat_;
  const int64_t sequence_;

  static AtomicInt64 s_numCreated_;

  DISABLE_COPY_AND_ASSIGN(Timer);
};

}  // namespace cobra

#endif  // COBRA_TIMER_H_
