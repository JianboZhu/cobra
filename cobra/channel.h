#ifndef COBRA_CHANNEL_H_
#define COBRA_CHANNEL_H_

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include "base/macros.h"
#include "base/timestamp.h"

namespace cobra {

class Worker;

// A selectable I/O channel.
//
// This class doesn't own the file descriptor.
// The file descriptor could be a socket,
// an eventfd, a timerfd, or a signalfd
class Channel {
 public:
  typedef boost::function<void()> EventCb;
  typedef boost::function<void(Timestamp)> ReadEventCb;

  Channel(Worker* loop, int fd);
  ~Channel();

  // The core interface of 'Channel'.
  // Handle the wakeuped event(maybe a read/write/error event etc) by
  // calling the registered callback functions.
  void handleEvent();
  void handleEvent(Timestamp receiveTime);

  // Cbs
  void setReadCb(const ReadEventCb& cb) {
    readCb_ = cb;
  }

  void setWriteCb(const EventCb& cb) {
    writeCb_ = cb;
  }

  void setCloseCb(const EventCb& cb) {
    closeCb_ = cb;
  }

  void setErrorCb(const EventCb& cb) {
    errorCb_ = cb;
  }

  // Tie this channel to the owner object managed by shared_ptr,
  // prevent the owner object being destroyed in handleEvent.
  void tie(const boost::shared_ptr<void>&);

  int fd() const { return fd_; }
  int events() const { return events_; }
  void set_revents(int revt) { revents_ = revt; } // used by pollers
  // int revents() const { return revents_; }
  bool isNoneEvent() const { return events_ == kNoneEvent; }

  void enableReading() {
    events_ |= kReadEvent;

    // This will call Worker::updateChannel, which then called
    // PollPoller::updateChannel.
    update();
  }
  // void disableReading() { events_ &= ~kReadEvent; update(); }
  void enableWriting() {
    events_ |= kWriteEvent;
    update();
  }
  void disableWriting() {
    events_ &= ~kWriteEvent;
    update();
  }
  void disableAll() {
    events_ = kNoneEvent;
    update();
  }
  bool isWriting() const { return events_ & kWriteEvent; }

  // For Poller
  int index() { return index_; }
  void set_index(int idx) { index_ = idx; }

  void doNotLogHup() { logHup_ = false; }

  Worker* ownerLoop() { return loop_; }
  void remove();

 private:
  void update();

  // Called by 'handleEvent' to do the real thing.
  void handleEventWithGuard();
  void handleEventWithGuard(Timestamp receiveTime);

  static const int kNoneEvent;
  static const int kReadEvent;
  static const int kWriteEvent;

  Worker* loop_;

  // The file descriptor, maybe a socket, an eventfd, signalfd or timerfd.
  const int  fd_;

  // a bit mask specifying the events the application is interested in
  // for the file descriptor fd_.
  int events_;

  // The field revents is an output parameter of 'poll', filled by the
  // kernel with the events that actually occurred. The bits returned
  // in revents can include any of those specified in events, or one of
  // the values POLLERR, POLLHUP, or POLLNVAL.
  int revents_;

  int index_; // used by Poller.
  bool logHup_;

  boost::weak_ptr<void> tie_;
  bool tied_;
  bool eventHandling_;

  ReadEventCb readCb_;
  EventCb writeCb_;
  EventCb closeCb_;
  EventCb errorCb_;

  DISABLE_COPY_AND_ASSIGN(Channel);
};

}  // namespace cobra

#endif  // COBRA_CHANNEL_H_
