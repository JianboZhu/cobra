#ifndef COBRA_BASE_ASYNCLOGGING_H_
#define COBRA_BASE_ASYNCLOGGING_H_

#include <cobra/base/BlockingQueue.h>
#include <cobra/base/BoundedBlockingQueue.h>
#include <cobra/base/CountDownLatch.h>
#include <cobra/base/Mutex.h>
#include <cobra/base/Thread.h>

#include <cobra/base/LogStream.h>

#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

namespace cobra
{

class AsyncLogging : boost::noncopyable
{
 public:

  AsyncLogging(const string& basename,
               size_t rollSize,
               int flushInterval = 3);

  ~AsyncLogging()
  {
    if (running_)
    {
      stop();
    }
  }

  void append(const char* logline, int len);

  void start()
  {
    running_ = true;
    thread_.start();
    latch_.wait();
  }

  void stop()
  {
    running_ = false;
    cond_.notify();
    thread_.join();
  }

 private:

  // declare but not define, prevent compiler-synthesized functions
  AsyncLogging(const AsyncLogging&);  // ptr_container
  void operator=(const AsyncLogging&);  // ptr_container

  void threadFunc();

  typedef cobra::detail::FixedBuffer<cobra::detail::kLargeBuffer> Buffer;
  typedef boost::ptr_vector<Buffer> BufferVector;
  typedef BufferVector::auto_type BufferPtr;

  const int flushInterval_;
  bool running_;
  string basename_;
  size_t rollSize_;
  cobra::Thread thread_;
  cobra::CountDownLatch latch_;
  cobra::MutexLock mutex_;
  cobra::Condition cond_;
  BufferPtr currentBuffer_;
  BufferPtr nextBuffer_;
  BufferVector buffers_;
};

}
#endif  // COBRA_BASE_ASYNCLOGGING_H_
