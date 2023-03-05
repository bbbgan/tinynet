// 
// Author       : gan
// Date         : 2022-12
// 
#ifndef TINYNET_BASE_ASYNCLOGGER_H_
#define TINYNET_BASE_ASYNCLOGGER_H_

#include <vector>
#include <memory>
#include <string>
#include <atomic>
#include <thread>

#include <tinynet/base/Logger.h>
#include <tinynet/base/CountDownLatch.h>

namespace tinynet
{
class AsyncLogger : noncopyable 
{
 public:
  AsyncLogger(const std::string& basename, int flushInterval = 3);
  ~AsyncLogger() {
    if (running_) {
      stop();
    }
  }
  void append(const char* logline, int len);
  void start();
  void stop() {
    running_ = false;
    cond_.notify_one();
    if (thread_.joinable()) {
      thread_.join();
    }
  }

 private:
  void threadFunc();
  using Buffer = detail::FixedBuffer<detail::kLargeBufferSize>;
  using BufferVector = std::vector<std::unique_ptr<Buffer>>;
  using BufferPtr = BufferVector::value_type;
  const std::string basename_;
  std::atomic<bool> running_;
  CountDownLatch latch_;
  BufferPtr currentBuffer_;
  BufferPtr nextBuffer_;
  BufferVector buffers_;
  std::mutex mtx_;
  std::condition_variable cond_;
  std::thread thread_;
};

} // namespace tinynet



#endif // TINYNET_BASE_ASYNCLOGGER_H_