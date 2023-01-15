// 
// Author       : gan
// Date         : 2022-09-02
// 
#include <signal.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <algorithm>

#include <tinynet/net/EventLoop.h>
#include <tinynet/base/Logger.h>

using namespace tinynet;
using namespace tinynet::net;

namespace {
__thread EventLoop* t_loopInThisThread = nullptr;
const int kPollTimeMs = 10000;

class IgnoreSigPipe {
 public:
  IgnoreSigPipe() { ::signal(SIGPIPE, SIG_IGN); }
};
IgnoreSigPipe initObj;

}  // namespace

EventLoop* EventLoop::getEventLoopOfCurrentThread() {
  return t_loopInThisThread;
}

EventLoop::EventLoop() 
    : looping_(false),
      quit_(false),
      eventHandling_(false),
      doingPendingTasks_(false),
      tid_(gettid()),
      poller_(new Poller(this)),
      timerQueue_(new TimerQueue(this)),
      wakeupFd_(::eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK)),
      wakeupChannel_(new Channel(this, wakeupFd_))
{
  if (wakeupFd_ == -1)
    LOG_FATAL("EventLoop::eventfd()");
  LOG_DEBUG("EventLoop created %p in thread %d", this, static_cast<int>(tid_));
  if (t_loopInThisThread != nullptr)
    LOG_FATAL("EventLoop %p already exist in this thread", t_loopInThisThread);
  t_loopInThisThread = this;
  wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
  wakeupChannel_->enableReading();
}

EventLoop::~EventLoop() {
  wakeupChannel_->disableAll();
  ::close(wakeupFd_);
  t_loopInThisThread = nullptr;
}

void EventLoop::loop() {
  assert(!looping_);
  assertInLoopThread();
  looping_ = true;
  quit_ = false;  
  LOG_TRACE("EventLoop %p start looping", this);
  while (!quit_) { 
    activeChannels_.clear();
    auto pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
    eventHandling_ = true;
    for (const auto& channel : activeChannels_) {
      channel->handleEvent(pollReturnTime_);
    }
    eventHandling_ = false;
    doPendingTasks();
  }
  LOG_TRACE("EventLoop %p quit", this);
  looping_ = false;
}

void EventLoop::quit() {
  assert(!quit_);
  quit_ = true;
  if (!isInLoopThread()) {
    wakeup();
  }
}

void EventLoop::runInLoop(Task cb) {
  if (isInLoopThread()) {
    cb();
  } else {
    queueInLoop(std::move(cb));
  }
}

void EventLoop::queueInLoop(Task cb) {
  {
    std::lock_guard<std::mutex> lk(mutex_);
    pendingTasks_.push_back(std::move(cb));
  }
  if (!isInLoopThread() || doingPendingTasks_) {
    wakeup();
  }
}
Timer* EventLoop::runAt(Timestamp time, Timer::TimerCallback cb) {
  return timerQueue_->addTimer(std::move(cb), time, Millisecond::zero());
}

Timer* EventLoop::runAfter(Nanosecond delay, Timer::TimerCallback cb) {
  return runAt(time::now() + delay, std::move(cb));
}

Timer* EventLoop::runEvery(Nanosecond interval, Timer::TimerCallback cb) {
  return timerQueue_->addTimer(std::move(cb), time::now() + interval, interval);
}

void EventLoop::cancel(Timer* timer) { timerQueue_->cancel(timer); }

void EventLoop::updateChannel(Channel* channel) {
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  poller_->updateChannel(channel);
}


void EventLoop::abortNotInLoopThread() {
  LOG_FATAL(
      "EventLoop::abortNotInLoopThread - EventLoop %p was created in threadId_ "
      "= %d, current thread id = %d",
      this, tid_, gettid());
}

void EventLoop::wakeup() {
  uint64_t one = 1;
  ssize_t n = ::write(wakeupFd_, &one, sizeof one);
  if (n != sizeof one) {
    LOG_ERROR("EventLoop::wakeup() should ::write() %lu bytes", sizeof(one));
  }
}

void EventLoop::handleRead() {
  uint64_t one = 1;
  ssize_t n = ::read(wakeupFd_, &one, sizeof one);
  if (n != sizeof one) {
    LOG_ERROR("EventLoop::handleRead() should ::read() %lu bytes", sizeof(one));
  }
}

void EventLoop::doPendingTasks() {
  assertInLoopThread();
  std::vector<Task> tasks;
  doingPendingTasks_ = true;
  {
    std::lock_guard<std::mutex> lk(mutex_);
    tasks.swap(pendingTasks_);
  }
  for (const auto& task : tasks) {
    task();
  }
  doingPendingTasks_ = false;
}
