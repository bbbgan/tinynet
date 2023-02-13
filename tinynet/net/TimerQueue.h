// 
// Author       : gan
// Date         : 2022-09
// 
#ifndef TINYNET_NET_TIMERQUEUE_H_
#define TINYNET_NET_TIMERQUEUE_H_

#include <set>
#include <memory>
#include <vector>

#include <tinynet/net/Channel.h>
#include <tinynet/net/Timer.h>
#include <tinynet/base/Timestamp.h>

namespace tinynet
{
namespace net
{
class EventLoop;
class Timer;
class TimerQueue : noncopyable
{
 public:
  explicit TimerQueue(EventLoop* loop);
  ~TimerQueue();
  Timer* addTimer(Timer::TimerCallback cb, Timestamp when, Nanosecond interval);
  void cancel(Timer* timer);
 private:
  typedef std::pair<Timestamp, Timer*> Entry;
  typedef std::set<Entry> TimerList;

  void addTimerInLoop(Timer* timer);
  void cancelInLoop(Timer* timer);
  void handleRead();
  std::vector<Entry> getExpired(Timestamp now);
  void reset(std::vector<Entry>& expired, Timestamp now);
  
 private:
  EventLoop* loop_;
  const int timerfd_;
  Channel timerfdChannel_;
  TimerList timers_;  
};

} // namespace net
} // namespace tinynet


#endif // TINTNET_NET_TIMERQUEUE_H_