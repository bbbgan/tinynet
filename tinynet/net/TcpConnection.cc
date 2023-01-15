// 
// Author       : gan
// Date         : 2022-09-05
//
#include <errno.h>

#include <tinynet/base/Logger.h>
#include <tinynet/net/TcpConnection.h>
#include <tinynet/net/EventLoop.h>

using namespace tinynet;
using namespace tinynet::net;
namespace tinynet 
{
namespace net 
{

void defaultThreadInitCallback(EventLoop* loop) {
  LOG_TRACE("EventLoop thread #%lu started", loop);
}
void defaultConnectionCallback(const TcpConnectionPtr& conn) {
  LOG_INFO("connection %s -> %s %s",
           conn->peerAddress().toIpPort().c_str(),
           conn->localAddress().toIpPort().c_str(),
           conn->connected() ? "up" : "down");
}
void defaultMessageCallback(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp stamp) {
  LOG_TRACE("connection %s -> %s recv %lu bytes",
            conn->peerAddress().toIpPort().c_str(),
            conn->localAddress().toIpPort().c_str(), 
            buffer->readableBytes());
  buffer->retrieveAll();
}
}  // namespace net
}  // namespace tinynet


TcpConnection::TcpConnection(EventLoop* loop, int sockfd,
                             const InetAddress& localAddr, 
                             const InetAddress& peerAddr,
                             const std::string& name)
    : loop_(loop),
      state_(kConnecting),
      socket_(new Socket(sockfd)),
      channel_(new Channel(loop, sockfd)),
      localAddr_(localAddr),
      peerAddr_(peerAddr),
      highWaterMark_(64 * 1024 * 1024),
      name_(name)
{
  channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this, _1));
  channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
  channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
  channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));
  LOG_TRACE("TcpConnection::TcpConnection() at %p fd=%d",this, socket_->fd());
  socket_->setKeepAlive(true);
}
TcpConnection::~TcpConnection() {
  LOG_TRACE("TcpConnection::~TcpConnection() at %p fd=%d",this, socket_->fd());
  assert(state_ == kDisconnected);
}

void TcpConnection::send(const void* data, int len) {
  if (state_ == kConnected) {
    if (loop_->isInLoopThread()) {
      sendInLoop(data, len);
    } else {
      loop_->queueInLoop(
          [ptr = shared_from_this(),
           str = std::string(static_cast<const char*>(data), 
                             static_cast<const char*>(data) + len)]() {
            ptr->sendInLoop(str);
          });
    }
  } else {
    LOG_WARN("this = %p TcpConnection::send() not connected, give up", this);
  }
}
void TcpConnection::send(const std::string& str) {
  if (state_ == kConnected) {
    if (loop_->isInLoopThread()) {
      sendInLoop(str);
    } else {
      loop_->queueInLoop([ptr = shared_from_this(), str]() {
            ptr->sendInLoop(str);
          });
    }
  } else {
    LOG_WARN("this = %p TcpConnection::send() not connected, give up", this);
  }  
}
void TcpConnection::send(Buffer* buf)
{
  if (state_ == kConnected)
  {
    if (loop_->isInLoopThread())
    {
      sendInLoop(buf->peek(), buf->readableBytes());
      buf->retrieveAll();
    }
    else
    {
      loop_->queueInLoop(
              [ptr = shared_from_this(), str = buf->retrieveAllAsString()]()
              { ptr->sendInLoop(str); });
    }
  } else {
    LOG_WARN("this = %p TcpConnection::send() not connected, give up", this);
  }
}

void TcpConnection::sendInLoop(const std::string& message) {
  sendInLoop(message.data(), message.size());
}

void TcpConnection::sendInLoop(const void* data, size_t len) {
  loop_->assertInLoopThread();
  ssize_t nwrote = 0;
  size_t remain = len;
  bool faultError = false;
  if (state_ == kDisconnected) {
    LOG_WARN("this = %p TcpConnection::send() not connected, give up", this);
    return;
  }
  // if no thing in output queue, try writing directly
  if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0)  {
    nwrote = ::write(channel_->fd(), data, len);
    if (nwrote >= 0) {
      remain -= static_cast<size_t>(nwrote);
      if (remain == 0 && writeCompleteCallback_) {
        // user may send data in writeCompleteCallback_
        // queueInLoop can break the chain
        loop_->queueInLoop(
            std::bind(writeCompleteCallback_, shared_from_this()));
      }
    } else {
      nwrote = 0;
      if (errno != EAGAIN) {
        LOG_ERROR("TcpConnectio::sendInLoop::write()");
        if (errno == EPIPE || errno == ECONNRESET)  // FIXME: any others?
        {  // EPIPE: reading end is closed; ECONNRESET: connection reset by
           // peer
          faultError = true;
        }
      }
    }
  }
  // it will quit the send when the product more and more faster than the consum 
  // oldLen < highWaterMark_   FIXME
  if (!faultError && remain > 0) {
    size_t oldLen = outputBuffer_.readableBytes();
    if (oldLen + remain >= highWaterMark_ 
        && oldLen < highWaterMark_  
        && highWaterMarkCallback_) {
      loop_->queueInLoop(std::bind(highWaterMarkCallback_, shared_from_this(),
                                   oldLen + remain));
    }  // append data to be written to the output buffer
    outputBuffer_.append(static_cast<const char*>(data) + nwrote, remain);
    if (!channel_->isWriting()) {  
      channel_->enableWriting();
    }
  }
}

void TcpConnection::shutdown() {
  // FIXME: use compare and swap
  if (stateAtomicGetAndSet(kDisconnecting) == kConnected) {
    if (loop_->isInLoopThread())
      shutdownInLoop();
    else {
      loop_->queueInLoop(
          std::bind(&TcpConnection::shutdownInLoop, shared_from_this()));
    }
  }
}


void TcpConnection::shutdownInLoop() {
  loop_->assertInLoopThread();
  if (!channel_->isWriting()) {
    // we are not writing
    socket_->shutdownWrite();
  }
}

void TcpConnection::forceClose() {
  if (state_ != kDisconnected) {
    if (stateAtomicGetAndSet(kDisconnecting) != kDisconnected) {
      loop_->queueInLoop(
          std::bind(&TcpConnection::forceCloseInLoop, shared_from_this()));
    }
  }
}

void TcpConnection::forceCloseInLoop() {
  loop_->assertInLoopThread();
  if (state_ != kDisconnected) {
    // as if we received 0 byte in handleRead();
    handleClose();  
  }
}

void TcpConnection::setTcpNoDelay(bool on) { socket_->setTcpNoDelay(on); }

void TcpConnection::startRead() {
  loop_->runInLoop([this]() {
    // FIXME: can I remove reading_;
    if (!channel_->isReading()) {
      channel_->enableReading();
    }
  });
}

void TcpConnection::stopRead() {
  loop_->runInLoop([this]() {
    if (channel_->isReading()) {
      channel_->disableReading();
    }
  });
}

int TcpConnection::stateAtomicGetAndSet(int newState)
{
    return __atomic_exchange_n(&state_, newState, __ATOMIC_SEQ_CST);
}

void TcpConnection::connectEstablished() { 
  loop_->assertInLoopThread();
  assert(state_ == kConnecting);
  state_ = kConnected;
  channel_->tie(shared_from_this());
  channel_->enableReading();
  connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed() {
  loop_->assertInLoopThread();
  if (state_ == kConnected) {
    state_ = kDisconnected;
    channel_->disableAll();
    connectionCallback_(shared_from_this());
  }
}
void TcpConnection::handleRead(Timestamp receiveTime) {
  loop_->assertInLoopThread();
  assert(state_ != kDisconnected);
  int savedErrno = 0;
  ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
  if (n > 0) {
    messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
  } else if (n == 0) {
    handleClose();
  } else {
    errno = savedErrno;
    LOG_ERROR("TcpConnection::handleRead");
    handleError();
  }
}

void TcpConnection::handleWrite() {
  if (state_ == kDisconnected) {
    LOG_WARN("TcpConnection::handleWrite() disconnect, give up");
    return;
  }
  loop_->assertInLoopThread();
  if (channel_->isWriting()) {
    ssize_t n = ::write(channel_->fd(), outputBuffer_.peek(),
                               outputBuffer_.readableBytes());
    if (n > 0) {
      outputBuffer_.retrieve(n);
      if (outputBuffer_.readableBytes() == 0) {
        channel_->disableWriting();
        if (writeCompleteCallback_) {
          loop_->queueInLoop(
              std::bind(writeCompleteCallback_, shared_from_this()));
        }
        if (state_ == kDisconnecting) {
          // this will make sure write completely
          shutdownInLoop();
        }
      }
    } else {
      LOG_ERROR("TcpConnection::handleWrite");
    }
  } else {
    LOG_TRACE("Connection fd = %d, is down, no more writing", channel_->fd());
  }
}

void TcpConnection::handleClose() {
  loop_->assertInLoopThread();
  assert(state_ == kConnected || state_ == kDisconnecting);
  // we don't close fd, leave it to dtor, so we can find leaks easily.
  state_ = kDisconnected;
  channel_->disableAll();
  TcpConnectionPtr guardThis(shared_from_this());
  connectionCallback_(guardThis);
  // must be the last line
  closeCallback_(guardThis);
}

void TcpConnection::handleError() {
  int err;
  socklen_t len = sizeof(err);
  int ret = getsockopt(channel_->fd(), SOL_SOCKET, SO_ERROR, &err, &len);
  if (ret != -1) errno = err;
  LOG_ERROR("TcpConnection::handleError() err: %d", err);
}
