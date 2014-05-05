#ifndef COBRA_NET_POLLER_POLLPOLLER_H_
#define COBRA_NET_POLLER_POLLPOLLER_H_

#include <cobra/net/Poller.h>

#include <map>
#include <vector>

struct pollfd;

namespace cobra
{
namespace net
{

///
/// IO Multiplexing with poll(2).
///
class PollPoller : public Poller
{
 public:

  PollPoller(EventLoop* loop);
  virtual ~PollPoller();

  virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels);

  // Update a channel.
  //   1. Adding a new channel.
  //   2. update an existing channel's warpped fd's 'events'(in the struct pollfd).
  virtual void updateChannel(Channel* channel);
  virtual void removeChannel(Channel* channel);

 private:
  void fillActiveChannels(int numEvents,
                          ChannelList* activeChannels) const;

  typedef std::vector<struct pollfd> PollFdList;
  PollFdList pollfds_;

  typedef std::map<int, Channel*> ChannelMap;
  ChannelMap channels_;
};

}
}
#endif  // COBRA_NET_POLLER_POLLPOLLER_H_
