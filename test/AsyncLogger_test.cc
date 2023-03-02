// 
// Author       : gan
// Date         : 2023-02
// 
#include <tinynet/base/AsyncLogger.h>
#include <tinynet/base/Logger.h>
#include <sys/time.h>

#include <stdio.h>
#include <sys/resource.h>
#include <unistd.h>


using namespace std;
using namespace tinynet;
AsyncLogger* gAsyncLog = NULL;
void asyncOutput(const char* msg, int len) {
  gAsyncLog->append(msg, len);
}

void bench() {
  
}
int main(int argc, char* argv[]) {
  printf("pid = %d\n", getpid());
  AsyncLogger log("tinynet_asynclog");
  
  log.start();
  gAsyncLog = &log;
  Logger::setLogLevel(Logger::LogLevel::TRACE);
  Logger::setOutput(asyncOutput);
  const char* buf = "Hello 0123456789  abcdefghijklmnopqrstuvwxyz";

  timeval t_start, t_end;
  gettimeofday(&t_start, NULL);

  for (int i =0; i < 1000; ++i) {
    LOG_TRACE("%s", buf);
    LOG_DEBUG("%s", buf);
    LOG_INFO("%s", buf);
    LOG_ERROR("%s", buf);
  }
  gettimeofday(&t_end, NULL);
  auto delta_t = (t_end.tv_sec - t_start.tv_sec)*1000000 +
                   (t_end.tv_usec - t_start.tv_usec);
  printf("per log cost  %lf us \n", static_cast<double>(delta_t)/4000.0);
  return 0;
}