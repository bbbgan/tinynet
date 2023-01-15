// 
// Author       : gan
// Date         : 2022-09-03
//
#include <tinynet/net/EventLoopThread.h>
#include <tinynet/net/EventLoop.h>

using namespace tinynet;
using namespace tinynet::net;

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb)
    : loop_(NULL), started_(false), callback_(cb), latch_(1) {}

EventLoopThread::~EventLoopThread() {
  if (started_) {
    if (loop_ != nullptr) {
      loop_->quit();
    }
    thread_.join();
  }
}

EventLoop* EventLoopThread::startLoop() {
  assert(!started_);
  started_ = true;
  assert(loop_ == nullptr);
  thread_ = std::thread([this]() { threadFunc(); });
  latch_.wait();
  assert(loop_ != nullptr);
  return loop_;
}

void EventLoopThread::threadFunc() {
  EventLoop loop;
  if (callback_) {
    callback_(&loop);
  }
  loop_ = &loop;
  latch_.countDown();
  loop.loop();
  loop_ = NULL;
}
