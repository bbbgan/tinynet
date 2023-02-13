// 
// Author       : gan
// Date         : 2022-08
// 
#include <tinynet/base/Logger.h>
#include <sys/time.h>

using namespace tinynet;
int main() {
  Logger::setLogLevel(Logger::LogLevel::TRACE);
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