#ifndef COBRA_NET_CHANNEL_H_
#define COBRA_NET_CHANNEL_H_

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include "base/macros.h"
#include "base/timestamp.h"

namespace cobra {

class EventLoop;

// A selectable I/O channel.
//
// This class doesn't own the file descriptor.
// The file descriptor could be a socket,
// an eventfd, a timerfd, or a signalfd
class Channel {
 public:
  typedef boost::function<void()> EventCallback;
  typedef boost::function<void(Timestamp)> ReadEventCallback;

  Channel(EventLoop* loop, int fd);
  ~Channel();

  // The core interface of 'Channel'.
  // Handle the wakeuped event(maybe a read/write/error event etc) by
  // calling the registered callback functions.
  void handleEvent();
  void handleEvent(Timestamp receiveTime);

  // Callbacks
  void setReadCallback(const ReadEventCallback& cb) {
    readCallback_ = cb;
  }

  void setWriteCallback(const EventCallback& cb) {
    writeCallback_ = cb;
  }

  void setCloseCallback(const EventCallback& cb) {
    closeCallback_ = cb;
  }

  void setErrorCallback(const EventCallback& cb) {
    errorCallback_ = cb;
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

    // This will call EventLoop::updateChannel, which then called
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

  // For debug
  string reventsToString() const;

  void doNotLogHup() { logHup_ = false; }

  EventLoop* ownerLoop() { return loop_; }
  void remove();

 private:
  void update();

  // Called by 'handleEvent' to do the real thing.
  void handleEventWithGuard();
  void handleEventWithGuard(Timestamp receiveTime);

  static const int kNoneEvent;
  static const int kReadEvent;
  static const int kWriteEvent;

  EventLoop* loop_;

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

  ReadEventCallback readCallback_;
  EventCallback writeCallback_;
  EventCallback closeCallback_;
  EventCallback errorCallback_;

  DISABLE_COPY_AND_ASSIGN(Channel);
};

}  // namespace cobra

#endif  // COBRA_NET_CHANNEL_H_
