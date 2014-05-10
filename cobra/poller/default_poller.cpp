#include "cobra/poller.h"
#include "cobra/poller/poll_poller.h"
#include "cobra/poller/epoll_poller.h"

#include <stdlib.h>

Poller* Poller::newDefaultPoller(Worker* loop) {
  if (::getenv("COBRA_USE_POLL")) {
    return new PollPoller(loop);
  } else {
    return new EPollPoller(loop);
  }
}
