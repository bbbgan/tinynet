// 
// Author       : gan
// Date         : 2022-09
//
#include <tinynet/net/EventLoop.h>
#include <tinynet/net/EventLoopThread.h>
#include <tinynet/net/EventLoopThreadPool.h>

using namespace tinynet;
using namespace tinynet::net;

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop)
    : baseLoop_(baseLoop),
      started_(false),
      numThreads_(0),
      next_(0) {}
EventLoopThreadPool::~EventLoopThreadPool() = default;
void EventLoopThreadPool::start(const ThreadInitCallback& cb) {
  assert(!started_);
  baseLoop_->assertInLoopThread();
  started_ = true;
  for (int i = 0; i < numThreads_; ++i) {
    EventLoopThread* t = new EventLoopThread(cb);
    threads_.push_back(std::unique_ptr<EventLoopThread>(t));
    loops_.push_back(t->startLoop());  
  }
  if (numThreads_ == 0 && cb) { 
    cb(baseLoop_);
  }
}

EventLoop* EventLoopThreadPool::getNextLoop() {
  baseLoop_->assertInLoopThread();
  assert(started_);
  EventLoop* loop = baseLoop_;
  if (!loops_.empty()) {
    loop = loops_[next_];
    ++next_;
    if (static_cast<size_t>(next_) >= loops_.size()) {
      next_ = 0;
    }
  }
  return loop;
}