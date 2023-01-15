// 
// Author       : gan
// Date         : 2022-09-04
// 
#include <tinynet/net/EventLoop.h>
#include <tinynet/net/EventLoopThread.h>
#include <tinynet/net/EventLoopThreadPool.h>
#include <tinynet/base/Logger.h>

#include <stdio.h>
#include <unistd.h>

using namespace tinynet;
using namespace tinynet::net;


int main() {  
  EventLoop baseloop;
  baseloop.runAfter(20s, std::bind(&EventLoop::quit, &baseloop));
  {
    EventLoopThreadPool loopPool_1(&baseloop);
    loopPool_1.start([](EventLoop*){
      LOG_INFO("single");
    });
    assert(&baseloop == loopPool_1.getNextLoop());
    assert(&baseloop == loopPool_1.getNextLoop());
  }
  {
    EventLoopThreadPool loopPool_2(&baseloop);
    loopPool_2.setThreadNum(3);
    loopPool_2.start([](EventLoop*){
      LOG_INFO("three threads");
    });
    for(int i = 0; i < 3; ++i) {
      auto loop = loopPool_2.getNextLoop();
      loop->runAfter(1s, [&loop](){
        LOG_INFO("Loop %p in thread %d", loop, gettid());
      });
      ::sleep(2);
    }
  }
  
  return 0;
}

