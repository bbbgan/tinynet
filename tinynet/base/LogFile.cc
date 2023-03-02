#include <tinynet/base/LogFile.h>

#include <assert.h>
#include <stdio.h>
#include <time.h>

using namespace tinynet;

void File::appendToFile(const char* line, const size_t len) {
  size_t haswrite = 0;
  while ( haswrite != len) {
    size_t remain = len - haswrite;
    size_t n = fwrite_unlocked(line + haswrite, 1, remain, fp_);
    if (n != remain) {
      // FIXME check the fp_
      assert(ferror(fp_));
    }
    haswrite += n;
  }
}


LogFile::LogFile(const std::string& basename)
  : basename_(basename),
    lastRoll_(0),
    startOfPeriod_(0)
{
  rollFile();
}

LogFile::~LogFile()  = default;

void LogFile::append(const char* logline, int len) {
  file_->appendToFile(logline, len);
  time_t now = ::time(NULL);
  time_t thisPeriod = now / (60 * 60 * 24);
  if (thisPeriod != startOfPeriod_) 
    rollFile();
  // the Logger response for the flush
}

void LogFile::rollFile() {
  time_t now = 0;
  std::string filename = getLogFileName(basename_, &now);
  startOfPeriod_ = now / (60 * 60 * 24);
  file_.reset(new File(filename.c_str()));
}

std::string LogFile::getLogFileName(const std::string& basename, time_t* now) {
  std::string filename;
  filename.reserve(basename.size() + 64);
  filename = basename;

  char timebuf[32];
  struct tm tm;
  *now = time(NULL);
  localtime_r(now, &tm); 
  strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S", &tm);
  filename += timebuf;
  filename += ".log";
  return filename;
}