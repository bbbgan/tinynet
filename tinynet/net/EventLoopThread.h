// 
// Author       : gan
// Date         : 2022-09
//
#ifndef TINYNET_NET_EVENTLOOPTHREAD_H_
#define TINYNET_NET_EVENTLOOPTHREAD_H_

#include <thread>
#include <functional>

#include <tinynet//base/noncopyable.h>
#include <tinynet//base/CountDownLatch.h>

namespace tinynet
{
namespace net
{
class EventLoop;
class EventLoopThread : noncopyable
{
 public:
  typedef std::function<void(EventLoop*)> ThreadInitCallback;
  EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback());
  ~EventLoopThread();
  EventLoop* startLoop();
 private:
  void threadFunc();
  // this value is the stack value in threadFunc
  EventLoop* loop_; 
  bool started_;
  ThreadInitCallback callback_;
  CountDownLatch latch_;
  std::thread thread_; 
};
} // namespace net

} // namespace tinynet


#endif // TINYNET_NET_EVENTLOOPTHREAD_H_
