#ifndef COBRA_POLLER_POLL_POLLER_H_
#define COBRA_POLLER_POLL_POLLER_H_

#include "cobra/poller.h"

#include <map>
#include <vector>

struct pollfd;

namespace cobra {

//
// IO Multiplexing with poll(2).
//
class PollPoller : public Poller {
 public:
  PollPoller(Worker* loop);
  virtual ~PollPoller();

  virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels);

  // Update a channel.
  //   1. Adding a new channel.
  //   2. update an existing channel's warpped fd's 'events'(in the struct pollfd).
  virtual void UpdateChannel(Channel* channel);
  virtual void removeChannel(Channel* channel);

 private:
  void fillActiveChannels(int numEvents,
                          ChannelList* activeChannels) const;

  typedef std::vector<struct pollfd> PollFdList;
  PollFdList pollfds_;

  typedef std::map<int, Channel*> ChannelMap;
  ChannelMap channels_;
};

}  // namespace cobra

#endif  // COBRA_POLLER_POLL_POLLER_H_
