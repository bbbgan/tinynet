// 
// Author       : gan
// Date         : 2022-09-02
// 
#include <sys/timerfd.h>
#include <unistd.h>
#include <strings.h>
#include <ratio> // std::nano::den

#include <tinynet/net/TimerQueue.h>
#include <tinynet/base/Logger.h>
#include <tinynet/net/EventLoop.h>

using namespace tinynet;
using namespace tinynet::net;

namespace {
struct timespec durationFromNow(Timestamp when) {
  struct timespec res;
  Nanosecond ns = when - time::now();
  if (ns < 1ms) ns = 1ms;
  res.tv_sec = static_cast<time_t>(ns.count() / std::nano::den);
  res.tv_nsec = ns.count() % std::nano::den;
  return res;
}

void resetTimerfd(int timerfd, Timestamp expiration) {
  struct itimerspec newValue, oldValue;
  bzero(&oldValue, sizeof(itimerspec));
  bzero(&newValue, sizeof(itimerspec));
  newValue.it_value = durationFromNow(expiration);
  int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
  if (ret) {
    LOG_ERROR("timerfd_settime()");
  }
}

void readTimerfd(int timerfd) {
  uint64_t val;
  ssize_t n = read(timerfd, &val, sizeof(val));
  if (n != sizeof(val)) LOG_ERROR("timerfdRead get %ld, not %lu", n, sizeof(val));
}

}  // namespace

TimerQueue::TimerQueue(EventLoop* loop)
    : loop_(loop),
      timerfd_(::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC)),
      timerfdChannel_(loop, timerfd_),
      timers_() {
  if (timerfd_ < 0)
    LOG_FATAL("TimerQueue::timerfd_create()");
  timerfdChannel_.setReadCallback(std::bind(&TimerQueue::handleRead, this));
  timerfdChannel_.enableReading();
}

TimerQueue::~TimerQueue() {
  timerfdChannel_.disableAll();
  ::close(timerfd_);
  for (auto& timer : timers_) {
    delete timer.second;
  }
}

Timer* TimerQueue::addTimer(Timer::TimerCallback cb, Timestamp when,
                            Nanosecond interval) 
{
  Timer* timer = new Timer(std::move(cb), when, interval);
  loop_->runInLoop(
      std::bind(&TimerQueue::addTimerInLoop, this, timer));  //线程安全
  return timer;
}

void TimerQueue::addTimerInLoop(Timer* timer) {
  loop_->assertInLoopThread();
  auto res = timers_.insert({timer->expiration(), timer});
  assert(res.second);
  if (timers_.begin() == res.first) {
    resetTimerfd(timerfd_, timer->expiration());
  }
}

void TimerQueue::cancel(Timer* timer) {
  loop_->runInLoop(std::bind(&TimerQueue::cancelInLoop, this, timer));
}

void TimerQueue::cancelInLoop(Timer* timer) {
  loop_->assertInLoopThread();
  timers_.erase({timer->expiration(), timer});
  delete timer; 
  // FIXEME: use shared_ptr
}

void TimerQueue::handleRead() {
  loop_->assertInLoopThread();
  Timestamp now(time::now());
  readTimerfd(timerfd_);
  std::vector<Entry> expired = getExpired(now);
  for (const auto& it : expired) {
    // warn : it may has been deleted by the previous task
    // try to use shared_ptr to fix it
    it.second->run();
  }
  reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now) {
  std::vector<Entry> expired;
  Entry sentry(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
  auto end = timers_.lower_bound(sentry);
  // assert(end == timers_.end() || now < end->first);
  std::copy(timers_.begin(), end, back_inserter(expired));
  timers_.erase(timers_.begin(), end);
  return expired;
}
void TimerQueue::reset(std::vector<Entry>& expired, Timestamp now) {
  Timestamp nextExpire;
  for (Entry& it : expired) {
    if (it.second->repeat()) {
      it.second->restart(now);
      it.first = it.second->expiration();
      timers_.insert(it);
    } else {
      // FIXME move to a free list
      delete it.second;  
    }
  }
  if (!timers_.empty()) {
    resetTimerfd(timerfd_, timers_.begin()->first);
  }
}