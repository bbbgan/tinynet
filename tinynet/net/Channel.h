// 
// Author       : gan
// Date         : 2022-08-27
// 
#ifndef TINYNET_NET_CHANNEL_H_
#define TINYNET_NET_CHANNEL_H_

#include <functional>
#include <memory>
#include <sys/epoll.h>

#include <tinynet/base/noncopyable.h>
#include <tinynet/base/Timestamp.h>

namespace tinynet 
{
namespace net 
{
class EventLoop;
// This class doesn't own the file descriptor.
// The file descriptor could be a socket,
// an eventfd, a timerfd, or a signalfd
// when we don't need Channel:
// please invoke disableAll()
class Channel : noncopyable {
 public:
  typedef std::function<void()> WriteCallback;
  typedef std::function<void()> ErrorCallback;
  typedef std::function<void()> CloseCallback;
  typedef std::function<void(Timestamp)> ReadCallback;

  Channel(EventLoop* loop, int fd);
  ~Channel();

  void handleEvent(Timestamp receiveTime);

  void setReadCallback(ReadCallback cb) { readCallback_ = std::move(cb); }
  void setWriteCallback(WriteCallback cb) { writeCallback_ = std::move(cb); }
  void setCloseCallback(CloseCallback cb) { closeCallback_ = std::move(cb); }
  void setErrorCallback(ErrorCallback cb) { errorCallback_ = std::move(cb); }
  
  void tie(const std::shared_ptr<void>&);
  int fd() const { return fd_; }
  int events() const { return events_; }
  void set_revents(int revent) { revents_ = revent; }
  bool isNoneEvent() const { return events_ == 0; }

  void enableReading() { events_ |= (EPOLLIN | EPOLLPRI); update(); }
  void disableReading() { events_ &= ~(EPOLLIN | EPOLLPRI); update(); }
  void enableWriting() { events_ |= EPOLLOUT; update(); }
  void disableWriting() { events_ &= ~EPOLLOUT; update(); }

  void disableAll() { events_ = 0; update(); }

  bool isWriting() const { return events_ & EPOLLOUT; }
  bool isReading() const { return events_ & (EPOLLIN | EPOLLPRI); }
  bool isPolling() const { return isPolling_; }

  void setPollingStatus(bool on) { isPolling_ = on; }
  EventLoop* ownerLoop() { return loop_; }

 private:
  void update();
  void handleEventWithGuard(Timestamp receiveTime);

  EventLoop* loop_;
  const int fd_;
  // it's the events we care about
  int events_;
  // it's the received event types of epoll or poll//revents_
  int revents_;
  std::weak_ptr<void> tie_;
  bool tied_;
  bool isPolling_;
  ReadCallback readCallback_;
  WriteCallback writeCallback_;
  CloseCallback closeCallback_;
  ErrorCallback errorCallback_;
};

}  // namespace net

}  // namespace tinynet

#endif  // TINYNET_NET_CHANNEL_H_