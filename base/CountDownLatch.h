#ifndef BASE_COUNTDOWNLATCH_H_
#define BASE_COUNTDOWNLATCH_H_

#include <base/Condition.h>
#include <base/Mutex.h>

#include <boost/noncopyable.hpp>

namespace cobra
{

class CountDownLatch : boost::noncopyable
{
 public:

  explicit CountDownLatch(int count);

  void wait();

  void countDown();

  int getCount() const;

 private:
  mutable MutexLock mutex_;
  Condition condition_;
  int count_;
};

}
#endif  // BASE_COUNTDOWNLATCH_H_
