// 
// Author       : gan
// Date         : 2022-08-26
// 
#ifndef TINYNET_NET_POLLER_H_
#define TINYNET_NET_POLLER_H_

#include <map>
#include <vector>

#include <tinynet/base/noncopyable.h>
#include <tinynet/base/Timestamp.h>
#include <tinynet/net/Channel.h>

namespace tinynet 
{
namespace net 
{
class EventLoop;
class Poller : noncopyable 
{
 public:
  typedef std::vector<Channel*> ChannelList;
  Poller(EventLoop* loop);
  ~Poller();
  Timestamp poll(int timeoutMs, ChannelList* activeChannels);
  void updateChannel(Channel* channel);
  
  
 private:
  typedef std::vector<struct epoll_event> EventList;
  void update(int operation, Channel* channel);

  EventLoop* ownerLoop_;
  int epollfd_;
  EventList events_;
  static const int kInitEventListSize = 32;
};

}  // namespace net
}  // namespace tinynet

#endif  // TINYNET_NET_POLLER_H_