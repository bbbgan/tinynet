// 
// Author       : gan
// Date         : 2022-09
// 
#include <tinynet/net/InetAddress.h>
#include <tinynet/net/EventLoop.h>
#include <tinynet/net/Acceptor.h>
#include <tinynet/base/Logger.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

using namespace tinynet;
using namespace tinynet::net;

namespace
{
int createSocket(sa_family_t family) {
  int ret = ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
  if (ret == -1) LOG_FATAL("Acceptor::socket()");
  else LOG_TRACE("create Socket Success");
  return ret;
}
}

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr,
                   bool reusePort)
    : loop_(loop),
      local_(listenAddr),
      acceptSocket_(createSocket(listenAddr.family())),
      acceptChannel_(loop, acceptSocket_.fd()),
      idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC))  
{
  assert(idleFd_ >= 0); // FIXME: CHECK
  acceptSocket_.setReuseAddr(true);
  acceptSocket_.setReusePort(reusePort);
  acceptSocket_.bindAddress(listenAddr);
  acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor() {
  acceptChannel_.disableAll();
  ::close(idleFd_);
}

void Acceptor::listen() {
  loop_->assertInLoopThread();
  acceptSocket_.listen();
  acceptChannel_.enableReading();
}

void Acceptor::handleRead() {
  loop_->assertInLoopThread();
  InetAddress peerAddr;
  // FIXME loop until no more
  int connfd = acceptSocket_.accept(&peerAddr);
  if (connfd >= 0) {
    if (newConnectionCallback_) {
      newConnectionCallback_(connfd, peerAddr);
    } else {
      LOG_WARN("there are not ConnectionCallback here");
      ::close(connfd);  // FIXME :check
    }
  } else {
    LOG_ERROR("Acceptor::handleRead()");
    if (errno == EMFILE) {
      ::close(idleFd_);
      idleFd_ = ::accept(acceptSocket_.fd(), NULL, NULL);
      ::close(idleFd_);
      idleFd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
    }
  }
}
