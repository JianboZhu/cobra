#ifndef COBRA_NET_POLLER_H
#define COBRA_NET_POLLER_H

#include <vector>

#include "base/macros.h"
#include "base/timestamp.h"
#include "cobra/event_loop.h"

namespace cobra {

class Channel;

// Base class for IO multiplexing
// Must be used in the loop thread.
class Poller {
 public:
  typedef std::vector<Channel*> ChannelList;

  Poller(EventLoop* loop) : ownerLoop_(loop) {
  }

  virtual ~Poller();

  /// Polls the I/O events.
  /// Must be called in the loop thread.
  virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels) = 0;

  /// Changes the interested I/O events.
  /// Must be called in the loop thread.
  virtual void updateChannel(Channel* channel) = 0;

  /// Remove the channel, when it des.
  /// Must be called in the loop thread.
  virtual void removeChannel(Channel* channel) = 0;

  static Poller* newDefaultPoller(EventLoop* loop);

  void assertInLoopThread() {
    ownerLoop_->assertInLoopThread();
  }

 private:
  EventLoop* ownerLoop_;
};

}  // namesapce cobra

#endif  // COBRA_NET_POLLER_H
