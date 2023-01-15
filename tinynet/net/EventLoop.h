// 
// Author       : gan
// Date         : 2022-09-02
// 
#ifndef TINYNET_NET_EVENTLOOP_H_
#define TINYNET_NET_EVENTLOOP_H_

#include <atomic>
#include <functional>
#include <vector>
#include <memory>
#include <mutex>
#include <unistd.h>

#include <tinynet/net/Channel.h>
#include <tinynet/net/Poller.h>
#include <tinynet/net/TimerQueue.h>
#include <tinynet/base/noncopyable.h>

namespace tinynet
{
namespace net
{
// Reactor, at most one per thread.
class EventLoop : noncopyable
{
 public:
  EventLoop();
  ~EventLoop();
  // loop forever
  void loop();
  // thread safe
  void quit();

  typedef std::function<void()> Task;
  void runInLoop(Task cb);
  void queueInLoop(Task cb);

  Timer* runAt(Timestamp when, Timer::TimerCallback cb);
  Timer* runAfter(Nanosecond delay, Timer::TimerCallback cb);
  Timer* runEvery(Nanosecond interval, Timer::TimerCallback cb);
  void cancel(Timer* timer);

  void wakeup();
  void updateChannel(Channel* channel);

  void assertInLoopThread() {
    if (!isInLoopThread()) {
      abortNotInLoopThread();
    }
  }
  bool isInLoopThread() const { return tid_ == gettid(); }
  bool eventHandling() const { return eventHandling_; }

  static EventLoop* getEventLoopOfCurrentThread();
 private:
  void abortNotInLoopThread();
  void handleRead();  // waked up
  void doPendingTasks();

  typedef std::vector<Channel*> ChannelList;

  bool looping_; 
  std::atomic<bool> quit_;
  bool eventHandling_; 
  bool doingPendingTasks_; 
  const pid_t tid_;
  std::unique_ptr<Poller> poller_;
  std::unique_ptr<TimerQueue> timerQueue_;
  int wakeupFd_;       
  std::unique_ptr<Channel> wakeupChannel_;
  Poller::ChannelList activeChannels_;  
  std::mutex mutex_;
  std::vector<Task> pendingTasks_; // guarded by mutex_
};

} // namespace net
} // namespace tinynet


#endif // TINYNET_NET_EVENTLOOP_H_