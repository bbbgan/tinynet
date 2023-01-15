// 
// Author       : gan
// Date         : 2022-09-04
//
#ifndef TINYNET_NET_EVENTLOOPTHREADPOOL_H_
#define TINYNET_NET_EVENTLOOPTHREADPOOL_H_

#include <tinynet//base/noncopyable.h>

#include <functional>

namespace tinynet
{
namespace net
{
class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : noncopyable 
{
 public:
  typedef std::function<void(EventLoop*)> ThreadInitCallback;
  EventLoopThreadPool(EventLoop* baseLoop);
  ~EventLoopThreadPool(); // Do not implement it in the header file
  void setThreadNum(int numThreads) { numThreads_ = numThreads; }
  void start(const ThreadInitCallback& cb = ThreadInitCallback());
  EventLoop* getNextLoop();
  bool started() const
  { return started_; }
 private:
  EventLoop* baseLoop_;
  bool started_; 
  int numThreads_; 
  int next_;
  std::vector<std::unique_ptr<EventLoopThread>> threads_;
  std::vector<EventLoop*> loops_;  
};


} // namespace net

} // namespace tinynet


#endif // TINYNET_NET_EVENTLOOPTHREADPOOL_H_
