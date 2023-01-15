// 
// Author       : gan
// Date         : 2022-09-01
// 
#ifndef TINYNET_BASE_COUNTDOWNLATCH_H_
#define TINYNET_BASE_COUNTDOWNLATCH_H_

#include <mutex>
#include <condition_variable>

#include <tinynet/base/noncopyable.h>

namespace tinynet
{
class CountDownLatch : noncopyable
{
 public:
  explicit CountDownLatch(int count) : count_(count) {}
  void wait() {
    std::unique_lock<std::mutex> lk(mutex_);
    while (count_ > 0) {
      cond_.wait(lk);
    }
  }
  void countDown() {
    std::lock_guard<std::mutex> lk(mutex_);
    --count_;
    if (count_ <= 0 ) 
      cond_.notify_all();
  }
 private:
  int count_;
  std::mutex mutex_;
  std::condition_variable cond_;
};

} // namespace tinynet


#endif // TINYNET_BASE_COUNTDOWNLATCH_H_