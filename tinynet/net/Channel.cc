// 
// Author       : gan
// Date         : 2022-08-27
// 
#include <assert.h>
#include <sstream>

#include <tinynet/net/Channel.h>
#include <tinynet/net/EventLoop.h>
#include <tinynet/base/Logger.h>

using namespace tinynet;
using namespace tinynet::net;

namespace
{

std::string eventsToString(int fd, int ev) {
  std::ostringstream oss;
  oss << fd << ": ";
  if (ev & EPOLLIN) oss << "IN ";
  if (ev & EPOLLPRI) oss << "PRI ";
  if (ev & EPOLLOUT) oss << "OUT ";
  if (ev & EPOLLHUP) oss << "HUP ";
  if (ev & EPOLLRDHUP) oss << "RDHUP ";
  if (ev & EPOLLERR) oss << "ERR ";
  return oss.str();
}
}

Channel::Channel(EventLoop* loop, int fd)
    : loop_(loop),
      fd_(fd),
      events_(0),
      revents_(0),
      tied_(false),
      isPolling_(false) {}

Channel::~Channel() {
  assert(!isPolling_);
}

void Channel::tie(const std::shared_ptr<void>& obj) {
  tie_ = obj;
  tied_ = true;
}

void Channel::update() {
  loop_->updateChannel(this);
}

void Channel::handleEvent(Timestamp receiveTime) {
  std::shared_ptr<void> guard;
  // TcpConnection is managed by std::shared_ptr,
  // and may be destructed when handling events,
  // so we use weak_ptr->shared_ptr to
  // extend it's life-time.
  if (tied_) {
    guard = tie_.lock();
    if (guard != nullptr) {
      handleEventWithGuard(receiveTime);
    }
  } else {
    handleEventWithGuard(receiveTime);
  }
}

void Channel::handleEventWithGuard(Timestamp receiveTime)
{
#ifndef NDEBUG
  LOG_INFO("Revent Loop[%p] fd : %d, revents : %s", loop_, fd_, eventsToString(fd_, revents_).c_str());
#endif
  if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN))
  {
    if (closeCallback_) closeCallback_();
  }

  if (revents_ & EPOLLERR)
  {
    if (errorCallback_) errorCallback_();
  }
  if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
  {
    if (readCallback_) readCallback_(receiveTime);
  }
  if (revents_ & EPOLLOUT)
  {
    if (writeCallback_) writeCallback_();
  }
}





