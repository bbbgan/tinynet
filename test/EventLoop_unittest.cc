// 
// Author       : gan
// Date         : 2022-09
// 
#include <assert.h>
#include <stdio.h>
#include <tinynet/base/Logger.h>
#include <tinynet/net/EventLoop.h>
#include <unistd.h>

#include <thread>

using namespace tinynet;
using namespace tinynet::net;

void anotherLoop() {
  EventLoop loop;
  loop.loop();
}
int main() {
  Logger::setLogLevel(Logger::LogLevel::INFO);
  assert(EventLoop::getEventLoopOfCurrentThread() == NULL);
  EventLoop loop;
  assert(EventLoop::getEventLoopOfCurrentThread() == &loop);

  std::thread thread(anotherLoop);
  auto every = loop.runEvery(1s, []() { });
  loop.runAfter(5s, [&]() {
    LOG_INFO("cancel the every event");
    loop.cancel(every);
  });
  loop.runAfter(10s, [&]() {
    LOG_INFO("loop will quit after 10s");
    loop.quit();
  });
  LOG_INFO("main loop start");
  loop.loop();
  LOG_INFO("main loop end");
  if (thread.joinable()) {
    thread.join();
  }
}
