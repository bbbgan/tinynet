// 
// Author       : gan
// Date         : 2022-08
// 
#ifndef TINYNET_BASE_LOGGER_H_
#define TINYNET_BASE_LOGGER_H_

#include <string.h>
#include <functional>
#include <sys/types.h> 
#include <unistd.h>	
#include <syscall.h>
#include <stdarg.h>

#include <tinynet/base/noncopyable.h>

namespace tinynet {
namespace detail {

const int kSmallBufferSize = 4000;  
const int kLargeBufferSize = 4000*1000;

template <int SIZE>
class FixedBuffer : noncopyable {
 public:
  FixedBuffer() : cur_(data_) {}
  ~FixedBuffer() = default;
  int length() const { return static_cast<int>(cur_ - data_); }
  char* current() { return cur_; }
  int avail() const { return static_cast<int>(sizeof data_ - length()); }
  void resetZero() { bzero(data_, sizeof data_); }
  void reset() { cur_ = data_; }
  const char* data() const { return data_; }
  
  // FIXEME:CHECK n
  void retrieve(size_t n) { cur_ += n; } 
  // len is as long as strlen
  void append(const char* buf, size_t len) {
    int remain = avail();
    if (static_cast<size_t>(avail()) > len) {
      memcpy(cur_, buf, len);
      cur_ += len;
    } else {
      memcpy(cur_, buf, avail() - 1);
      cur_ += remain - 1;      
    }
  }
  void appendNum(int number) {
    size_t nwrote = static_cast<size_t>(snprintf(current(), avail(), "%d ", number));
    retrieve(nwrote);
  }
 private:
  char data_[SIZE];
  char* cur_;
};

// get the filename during the compile time
constexpr const char* str_end(const char* str) {
  return *str ? str_end(str + 1) : str;
}
constexpr const char* last_slant(const char* str) {
  return *str == '/' ? str + 1 : last_slant(str - 1);
} 
constexpr bool has_slant(const char* str) {
  return *str == '/' ? true : (*str ? has_slant(str + 1) : false);
}
constexpr const char* staticGetFilename(const char* str) {
  return has_slant(str) ? last_slant(str_end(str)) : str;
}
constexpr int staticGetLength(const char* str) {
  return *str ?  staticGetLength(str + 1) + 1 : 1;
}

struct BaseName {
  constexpr BaseName(const char* filename) 
    : data_(staticGetFilename(filename)),
      len_(staticGetLength(data_)) {}
  const char* data_;
  int len_;
};

}  // namespace detail

class Logger {
 public:
  typedef detail::FixedBuffer<detail::kSmallBufferSize> Buffer;
  typedef std::function<void(const char*, int)> OutputFunc;
  typedef std::function<void()> FlushFunc;
  enum LogLevel {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
    NUM_LOG_LEVELS,
  };
  static LogLevel glevel_;
  static OutputFunc goutput_;
  static FlushFunc gflush_;
  
  Logger(const detail::BaseName file, int line, LogLevel level, const char* format, ...);
  ~Logger();
  static void setLogLevel(LogLevel level) { glevel_ = level; }
  static LogLevel getLogLevel() { return glevel_;}
  static void setOutput(const OutputFunc& func) { goutput_ = func; }
  static void setFlush(FlushFunc& func) { gflush_ = func; } 
  
 private:
  void formatTime();
  void finish();
  detail::BaseName basename_;
  int line_;
  LogLevel level_;
  Buffer buffer_;
};
using tinynet::Logger;
// user api
#define LOG_TRACE(format, ...) if (Logger::glevel_ <= Logger::TRACE) \
  Logger(__FILE__, __LINE__, Logger::TRACE, format, ##__VA_ARGS__)
#define LOG_DEBUG(format, ...) if (Logger::glevel_ <= Logger::DEBUG) \
  Logger(__FILE__, __LINE__, Logger::DEBUG, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...) if (Logger::glevel_ <= Logger::INFO) \
  Logger(__FILE__, __LINE__, Logger::INFO, format, ##__VA_ARGS__)
#define LOG_WARN(format, ...) Logger(__FILE__, __LINE__, Logger::WARN, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) Logger(__FILE__, __LINE__, Logger::ERROR, format, ##__VA_ARGS__)
#define LOG_FATAL(format, ...) Logger(__FILE__, __LINE__, Logger::FATAL, format, ##__VA_ARGS__)

}  // namespace tinynet
#endif  // TINYNET_BASE_LOGGER_H_
