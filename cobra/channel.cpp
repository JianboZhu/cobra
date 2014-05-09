#include "cobra/channel.h"

#include <poll.h>
#include <cstdio>

#include "base/Logging.h"
#include "cobra/worker.h"

namespace cobra {

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(Worker* loop, int fd)
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

  if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) {
    if (logHup_) {
      LOG_WARN << "Channel::handle_event() POLLHUP";
    }
    // @see TcpConnection::handleClose if the wrapped fd is a conn socket
    if (closeCb_) {
      closeCb_();
    }
  }

  if (revents_ & POLLNVAL) {
    LOG_WARN << "Channel::handle_event() POLLNVAL";
  }

  if ((revents_ & (POLLERR | POLLNVAL)) && errorCb_) {
    // @see TcpConnection::handleError if the wrapped fd is a conn socket
    errorCb_();
  }
  if ((revents_ & (POLLIN | POLLPRI | POLLRDHUP)) && readCb_) {
    // @see TcpConnection::handleRead if the wrapped fd is a conn socket
    readCb_(receiveTime);
  }
  if ((revents_ & POLLOUT) && writeCb_) {
    // @see TcpConnection::handleWrite if the wrapped fd is a conn socket
    writeCb_();
  }

  eventHandling_ = false;
}

}  // namespace cobra
