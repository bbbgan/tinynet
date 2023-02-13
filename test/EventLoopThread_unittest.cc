// 
// Author       : gan
// Date         : 2022-09
// 
#include <tinynet/net/EventLoopThread.h>
#include <tinynet/net/EventLoop.h>
#include <tinynet/base/Logger.h>

#include <stdio.h>
#include <unistd.h>

using namespace tinynet;
using namespace tinynet::net;

int main() {
  Logger::setLogLevel(Logger::LogLevel::INFO);
   // never start
  {
    EventLoopThread thr1;
  }
  {
    EventLoopThread thr2([](EventLoop* loop){
      LOG_INFO("EventLoop %p is in thread", loop);
    });
    thr2.startLoop();
  }
}

