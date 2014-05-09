#include <cobra/Poller.h>
#include <cobra/poller/PollPoller.h>
#include <cobra/poller/EPollPoller.h>

#include <stdlib.h>

Poller* Poller::newDefaultPoller(Worker* loop) {
  if (::getenv("COBRA_USE_POLL"))
  {
    return new PollPoller(loop);
  }
  else
  {
    return new EPollPoller(loop);
  }
}
