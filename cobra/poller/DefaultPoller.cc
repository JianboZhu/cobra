#include <cobra/net/Poller.h>
#include <cobra/net/poller/PollPoller.h>
#include <cobra/net/poller/EPollPoller.h>

#include <stdlib.h>

using namespace cobra::net;

Poller* Poller::newDefaultPoller(EventLoop* loop)
{
  if (::getenv("COBRA_USE_POLL"))
  {
    return new PollPoller(loop);
  }
  else
  {
    return new EPollPoller(loop);
  }
}
