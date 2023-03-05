// 
// Author       : gan
// Date         : 2022-12
// 
#ifndef TINYNET_BASE_LOGFILE_H
#define TINYNET_BASE_LOGFILE_H

#include <memory>
#include <string>
#include <mutex>
#include <sys/types.h> 
#include <assert.h>
#include <string>

#include <tinynet/base/noncopyable.h>

namespace tinynet
{
class File {
 public:
  File(const char* filename)
    : fp_(::fopen(filename, "ae"))
  {
    // FIXME check fp_
    assert(fp_);
    ::setbuffer(fp_, buffer_, sizeof buffer_);
  }
  ~File() { ::fclose(fp_); }
  // thread unsafe
  void appendToFile(const char* line, const size_t len);
  void flush() { ::fflush(fp_); }
 private:
  FILE* fp_;
  char buffer_[64 * 1024];
};

class LogFile : noncopyable
{
 public:
  LogFile(const std::string& basename);
  ~LogFile();
  // thread unsafe
  void append(const char* logline, int len);
  void flush() { file_->flush(); }
private:
  void rollFile();
  static std::string getLogFileName(const std::string& basename, time_t* now);
  const std::string basename_;
  time_t lastRoll_;
  time_t startOfPeriod_;
  std::unique_ptr<File> file_;

  std::mutex mtx_;
};

} // namespace tinynet


#endif // TINYNET_BASE_LOGFILE_H