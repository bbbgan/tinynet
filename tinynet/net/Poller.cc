// 
// Author       : gan
// Date         : 2022-08-26
// 
#include <assert.h>
#include <errno.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <tinynet/net/Poller.h>
#include <tinynet/base/Timestamp.h>
#include <tinynet/base/Logger.h>
#include <tinynet/net/EventLoop.h>

using namespace tinynet;
using namespace tinynet::net;

Poller::Poller(EventLoop* loop)
    : ownerLoop_(loop),
      epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
      events_(kInitEventListSize) 
{
  if (epollfd_ < 0) {
    LOG_FATAL("Poller::epoll_create1()");
  }
}

Poller::~Poller() {
  ::close(epollfd_);
}

Timestamp Poller::poll(int timeoutMs, ChannelList* activeChannels) {
  int numEvents = ::epoll_wait(epollfd_,
                               events_.data(),
                               static_cast<int>(events_.size()), 
                               timeoutMs);
  if (numEvents == -1) {
    if (errno != EINTR)
      LOG_ERROR("EPoller::epoll_wait()");   
  } else if (numEvents > 0) {
    LOG_TRACE("%d events happened", numEvents);
    // fill activeChannels
    for (int i = 0; i < numEvents; ++i) {
      Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
      channel->set_revents(events_[i].events);
      activeChannels->push_back(channel);
    }
    if (static_cast<size_t>(numEvents) == events_.size()) {
      events_.resize(2 * events_.size());
    }
  }
  Timestamp now(time::now());
  return now;
}


void Poller::updateChannel(Channel* channel) {
  ownerLoop_->assertInLoopThread();
  int operation = 0;
  if (!channel->isPolling()) {
    assert(!channel->isNoneEvent());
    operation = EPOLL_CTL_ADD;
    channel->setPollingStatus(true);
  } else if (channel->isNoneEvent()) {
    operation = EPOLL_CTL_DEL;
    channel->setPollingStatus(false);
  } else {
    operation = EPOLL_CTL_MOD;
  }
  update(operation, channel);
}

void Poller::update(int operation, Channel* channel) {
  struct epoll_event event;
  bzero(&event, sizeof event);
  event.events = channel->events();
  event.data.ptr = channel;
  int ret = ::epoll_ctl(epollfd_, operation, channel->fd(), &event);
  if (ret == -1)
    LOG_FATAL("POLLER::epoll_ctl()");
}