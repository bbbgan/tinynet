// 
// Author       : gan
// Date         : 2022-09-07
//
#include <tinynet/net/TcpServer.h>
#include <tinynet/net/EventLoop.h>
#include <tinynet/net/EventLoopThreadPool.h>
#include <tinynet/net/Acceptor.h>
#include <tinynet/base/Logger.h>

using namespace tinynet;
using namespace tinynet::net;
using std::string;

TcpServer::TcpServer(EventLoop* loop, const InetAddress& local,
                     const std::string& name, bool reuseport)
    : loop_(loop),
      localAddr_(local),
      ipPort_(local.toIpPort()),
      name_(name),
      acceptor_(new Acceptor(loop, local, reuseport)),
      threadPool_(new EventLoopThreadPool(loop)),
      connectionCallback_(defaultConnectionCallback),
      messageCallback_(defaultMessageCallback),
      started_(false), 
      nextConnId_(0)
      { acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this, _1, _2)); }

TcpServer::~TcpServer() {
  loop_->assertInLoopThread();
  LOG_TRACE("TcpServer::~TcpServer[%s]", name_);
  for (auto& it : connMap_) {
    TcpConnectionPtr conn(it.second);
    it.second.reset();
    conn->getLoop()->runInLoop(
      std::bind(&TcpConnection::connectDestroyed, conn));
  }
}

void TcpServer::setThreadNum(int numThreads) {
  assert(0 <= numThreads);
  assert(!started_);
  threadPool_->setThreadNum(numThreads);
}

void TcpServer::start() {
  if (started_.exchange(true) == false) {
    threadPool_->start(threadInitCallback_);
    LOG_INFO("TcpServer[%s] listen start : %s", name_.c_str(), localAddr_.toIpPort().c_str());
    loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
  }
}

void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr) {
  loop_->assertInLoopThread();
  EventLoop* ioLoop = threadPool_->getNextLoop();
  char buf[64];
  ++nextConnId_;
  snprintf(buf, sizeof buf, "-%s#%d", ipPort_.c_str(), nextConnId_);
  string connName = name_ + buf;
  LOG_INFO("TcpServer[%s]::newConnection - new connection[%s] from %s",
            name_.c_str(), connName.c_str(), peerAddr.toIpPort().c_str());
  // FIXME poll with zero timeout to double confirm the new connection
  // FIXME use make_shared if necessary
  TcpConnectionPtr conn(new TcpConnection(ioLoop, sockfd, localAddr_, peerAddr, connName));
  connMap_[connName] = conn;
  conn->setConnectionCallback(connectionCallback_);
  conn->setMessageCallback(messageCallback_);
  conn->setWriteCompleteCallback(writeCompleteCallback_);
  conn->setCloseCallback(
      std::bind(&TcpServer::removeConnection, this, _1));  // FIXME: unsafe
  ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn) {
  // FIXME: unsafe
  loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn) {
  loop_->assertInLoopThread();
  connMap_.erase(conn->name());
}
