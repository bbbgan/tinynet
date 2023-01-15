// 
// Author       : gan
// Date         : 2022-09-02
// 
#ifndef TINYNET_NET_TIMER_H_
#define TINYNET_NET_TIMER_H_

#include <assert.h>

#include <functional>
#include <tinynet/base/noncopyable.h>

namespace tinynet 
{
namespace net 
{
class Timer : noncopyable {
 public:
  typedef std::function<void()> TimerCallback;
  Timer(TimerCallback cb, Timestamp when, Nanosecond interval)
      : callback_(std::move(cb)),
        expiration_(when),
        interval_(interval),
        repeat_(interval > Nanosecond::zero()) {}
  void run() const { if (callback_) callback_(); }
  Timestamp expiration() const { return expiration_; }
  bool repeat() const { return repeat_; }
  void restart(Timestamp now) {
    assert(repeat_);
    expiration_ += interval_;
  }

 private:
  const TimerCallback callback_;
  Timestamp expiration_;
  const Nanosecond interval_;
  const bool repeat_;
};
}  // namespace net
}  // namespace tinynet

#endif  // TINYNET_NET_TIMER_H_