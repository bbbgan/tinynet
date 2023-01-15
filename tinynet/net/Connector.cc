// 
// Author       : gan
// Date         : 2022-09-10
//
#include <errno.h>
#include <tinynet/base/Logger.h>
#include <tinynet/net/Connector.h>
#include <tinynet/net/EventLoop.h>
#include <tinynet/net/Socket.h>
#include <tinynet/base/Timestamp.h>

using namespace tinynet;
using namespace tinynet::net;

static const int kMaxRetryDelayMs = 3*1000;
static const int kInitRetryDelayMs = 500;

namespace {
int getSocketError(int sockfd) {
  int optval;
  socklen_t optlen = static_cast<socklen_t>(sizeof optval);
  if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
    return errno;
  } else {
    return optval;
  }
}

}  // namespace

Connector::Connector(EventLoop* loop, const InetAddress& peerAddr)
    : loop_(loop),
      peerAddr_(peerAddr),
      startconnect_(false),
      retryDelayMs_(kInitRetryDelayMs),
      state_(kDisconnected) {
  LOG_TRACE("Connector::Connector()");
}

Connector::~Connector() {
  LOG_TRACE("Connector::~Connector()");
  assert(!channel_);
}

void Connector::start() {
  startconnect_ = true;
  loop_->runInLoop(std::bind(&Connector::startInLoop, this));  // FIXME: unsafe
}

void Connector::startInLoop() {
  assert(state_ == kDisconnected);
  loop_->assertInLoopThread();
  if (startconnect_) {
    connect();
  } else {
    LOG_INFO("connector[%p] do not start connect", this);
  }
}

void Connector::connect() {
  int sockfd = ::socket(peerAddr_.family(),
                        SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
  if (sockfd == -1) {
    LOG_FATAL("Connector::connect()");
  }
  int ret = ::connect(sockfd, peerAddr_.getSockAddr(), peerAddr_.getSocklen());
  int savedErrno = (ret == 0) ? 0 : errno;
  LOG_TRACE("connect Errno: %d", savedErrno);
  if (savedErrno == EINPROGRESS || savedErrno == 0) {
    connecting(sockfd);
  } else {
    retry(sockfd);
  }
}

void Connector::connecting(int sockfd) {
  stateAtomicGetAndSet(kConnecting);
  assert(!channel_);
  LOG_TRACE("connecting");
  channel_.reset(new Channel(loop_, sockfd));
  channel_->setWriteCallback(
      std::bind(&Connector::handleWrite, this));  // FIXME: unsafe
  channel_->setErrorCallback(
      std::bind(&Connector::handleError, this));  // FIXME: unsafe
  // channel_->tie(shared_from_this()); is not working,
  // as channel_ is not managed by shared_ptr FIXEME
  if (!channel_->isWriting())
    channel_->enableWriting();
}


void Connector::handleWrite() {
  LOG_ERROR("Connector::handleWrite()");
  if (state_ == kConnecting) {
    int sockfd = removeAndResetChannel();
    int err = getSocketError(sockfd);
    if (err) {
      retry(sockfd);
    } else {
      stateAtomicGetAndSet(kConnected);
      if (startconnect_) {
        newConnectionCallback_(sockfd, peerAddr_);
      } else {
        ::close(sockfd);
      }
    }
  }
}


int Connector::removeAndResetChannel() {
  channel_->disableAll();  
  int sockfd = channel_->fd();
  // Can't reset channel_ here, because we are inside Channel::handleEvent
  loop_->queueInLoop([this]() { channel_.reset(); });  // FIXME: unsafe
  return sockfd;
}

void Connector::handleError() {
  LOG_ERROR("Connector::handleError()");
  if (state_ == kConnecting) {
    int sockfd = removeAndResetChannel();
    int err = getSocketError(sockfd);
    LOG_TRACE("SO_ERROR = %d ", err);
    retry(sockfd);
  }

}

void Connector::retry(int sockfd) {
  ::close(sockfd);
  stateAtomicGetAndSet(kDisconnected);
  if (startconnect_) {
    LOG_INFO("Connector::retry - Retry connecting to [%s] in %d milliseconds",
              peerAddr_.toIpPort().c_str(), retryDelayMs_);
    loop_->runAfter(Millisecond(retryDelayMs_),
                    std::bind(&Connector::startInLoop, shared_from_this()));
    retryDelayMs_ = std::min(retryDelayMs_ * 2, kMaxRetryDelayMs);
  } else {
    LOG_INFO("connector[%p] do not start connect", this);
  }
}
