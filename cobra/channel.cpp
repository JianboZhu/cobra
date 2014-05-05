#include "cobra/channel.h"

#include <poll.h>
#include <cstdio>

#include "base/Logging.h"
#include "cobra/event_loop.h"

namespace cobra {

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, int fd)
  : loop_(loop),
    fd_(fd),
    events_(0),
    revents_(0),
    index_(-1),
    logHup_(true),
    tied_(false),
    eventHandling_(false) {
}

Channel::~Channel() {
  assert(!eventHandling_);
}

void Channel::tie(const boost::shared_ptr<void>& obj) {
  tie_ = obj;
  tied_ = true;
}

void Channel::update() {
  loop_->updateChannel(this);
}

void Channel::remove() {
  assert(isNoneEvent());
  loop_->removeChannel(this);
}

void Channel::handleEvent(Timestamp receiveTime) {
  boost::shared_ptr<void> guard;
  if (tied_) {
    guard = tie_.lock();
    if (guard) {
      handleEventWithGuard(receiveTime);
    }
  } else {
    handleEventWithGuard(receiveTime);
  }
}

void Channel::handleEventWithGuard(Timestamp receiveTime) {
  eventHandling_ = true;

  LOG_TRACE << reventsToString();
  if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) {
    if (logHup_) {
      LOG_WARN << "Channel::handle_event() POLLHUP";
    }
    // @see TcpConnection::handleClose if the wrapped fd is a conn socket
    if (closeCallback_) {
      closeCallback_();
    }
  }

  if (revents_ & POLLNVAL) {
    LOG_WARN << "Channel::handle_event() POLLNVAL";
  }

  if ((revents_ & (POLLERR | POLLNVAL)) && errorCallback_) {
    // @see TcpConnection::handleError if the wrapped fd is a conn socket
    errorCallback_();
  }
  if ((revents_ & (POLLIN | POLLPRI | POLLRDHUP)) && readCallback_) {
    // @see TcpConnection::handleRead if the wrapped fd is a conn socket
    readCallback_(receiveTime);
  }
  if ((revents_ & POLLOUT) && writeCallback_) {
    // @see TcpConnection::handleWrite if the wrapped fd is a conn socket
    writeCallback_();
  }

  eventHandling_ = false;
}

string Channel::reventsToString() const {
  string debug_string;
  char buffer[8];
  sprintf(buffer, "%d", fd_);

  debug_string.append(buffer).append(": ");
  if (revents_ & POLLIN) {
    debug_string.append("in ");
  }
  if (revents_ & POLLPRI) {
    debug_string.append("pri ");
  }
  if (revents_ & POLLOUT) {
    debug_string.append("out ");
  }
  if (revents_ & POLLHUP) {
    debug_string.append("hup ");
  }
  if (revents_ & POLLRDHUP) {
    debug_string.append("rdhup ");
  }
  if (revents_ & POLLERR) {
    debug_string.append("err ");
  }
  if (revents_ & POLLNVAL) {
    debug_string.append("nval");
  }

  return debug_string;
}

}  // namespace net
