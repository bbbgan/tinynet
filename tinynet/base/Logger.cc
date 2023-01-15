// 
// Author       : gan
// Date         : 2022-08-23
// 
#include <sstream>
#include <sys/time.h>

#include <tinynet/base/Logger.h>

#define UTCTOCST 28800
namespace tinynet
{
const char* LogLevelName[Logger::NUM_LOG_LEVELS] =
{
  "TRACE ",
  "DEBUG ",
  "INFO  ",
  "WARN  ",
  "ERROR ",
  "FATAL ",
};

void defaultOutput(const char* msg, int len)  //默认输出到stdout
{
  fwrite(msg, 1, len, stdout);
}

void defaultFlush() {
  fflush(stdout);  
}

} // namespace tinynet

using namespace tinynet;

Logger::LogLevel Logger::glevel_ = Logger::INFO;
Logger::FlushFunc Logger::gflush_ = defaultFlush;
Logger::OutputFunc Logger::goutput_ = defaultOutput;

Logger::~Logger() {
  finish();
  goutput_(buffer_.data(), buffer_.length());
  if (level_ == FATAL) {
    gflush_();
    abort();
  }
}


Logger::Logger(const detail::BaseName file, int line, LogLevel level, const char* format, ...)
  : basename_(file),
    line_(line),
    level_(level)
{
  va_list args_;
  va_start(args_, format);
  formatTime();
  buffer_.appendNum(static_cast<int>(gettid()));
  buffer_.append(LogLevelName[level_], 6);
  int nwrote = vsnprintf(buffer_.current(), buffer_.avail(), format, args_);
  va_end(args_);
  buffer_.retrieve(nwrote);
}


void Logger::formatTime() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  time_t seconds = tv.tv_sec + UTCTOCST;
  struct tm tm_time;
  gmtime_r(&seconds, &tm_time);
  size_t nwrote = static_cast<size_t>(snprintf(
                  buffer_.current(), buffer_.avail(), "%4d%02d%02d %02d:%02d:%02d.%06ld ",
                  tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                  tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, tv.tv_usec));
  buffer_.retrieve(nwrote);
}

void Logger::finish() {
  buffer_.append(" - ", 3);
  buffer_.append(basename_.data_, basename_.len_ - 1);
  buffer_.append(":", 1);
  buffer_.appendNum(line_);
  buffer_.append("\n", 1);
}


