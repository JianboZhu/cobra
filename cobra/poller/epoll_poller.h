#ifndef COBRA_POLLER_EPOLL_POLLER_H_
#define COBRA_POLLER_EPOLL_POLLER_H_

#include "cobra/poller.h"

#include <map>
#include <vector>

struct epoll_event;

namespace cobra {

///
/// IO Multiplexing with epoll(4).
///
class EPollPoller : public Poller
{
 public:
  EPollPoller(Worker* loop);
  virtual ~EPollPoller();

  virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels);
  virtual void UpdateChannel(Channel* channel);
  virtual void removeChannel(Channel* channel);

 private:
  static const int kInitEventListSize = 16;

  void fillActiveChannels(int numEvents,
                          ChannelList* activeChannels) const;
  void update(int operation, Channel* channel);

  typedef std::vector<struct epoll_event> EventList;
  typedef std::map<int, Channel*> ChannelMap;

  int epollfd_;
  EventList events_;
  ChannelMap channels_;
};

}  // namespace cobra

#endif  // COBRA_POLLER_EPOLL_POLLER_H_
