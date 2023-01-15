// 
// Author       : gan
// Date         : 2022-09-011
//
#include <tinynet/net/TcpClient.h>
#include <tinynet/net/Connector.h>
#include <tinynet/net/EventLoop.h>
#include <tinynet/base/Logger.h>

using namespace tinynet;
using namespace tinynet::net;


TcpClient::TcpClient(EventLoop* loop,
                     const InetAddress& peerAddr)
  : loop_(loop),
    connector_(new Connector(loop, peerAddr)),
    connectionCallback_(defaultConnectionCallback),
    messageCallback_(defaultMessageCallback),
    startconnect_(false)
{
  connector_->setNewConnectionCallback(
      std::bind(&TcpClient::newConnection, this, _1, _2));
  // FIXME setConnectFailedCallback
  LOG_TRACE("TcpClient::TcpClient()[%p]", this);
}

TcpClient::~TcpClient() {
  // when the connection established, the source wil free automatic
  if (connection_ && !connection_->disconnected())  // if connection established
    loop_->queueInLoop(std::bind(&TcpConnection::connectDestroyed, connection_));
  else
    ::close(connector_->removeAndResetChannel());
    // remove source
}

void TcpClient::startconnect() {
  LOG_INFO("TcpClient::connect[%p] - start connecting to [%s]", this,
           connector_->peerAddress().toIpPort().c_str());
  // FIXME: check state
  startconnect_ = true;
  connector_->start();
}

void TcpClient::disconnect() {
  startconnect_ = false;
  if (connection_) {
    connection_->shutdown();

  }
}

void TcpClient::newConnection(int sockfd, const InetAddress& peerAddr) {
  loop_->assertInLoopThread();
  char buf[32];
  snprintf(buf, sizeof buf, ":%s", peerAddr.toIpPort().c_str());

  struct sockaddr_in addr;
  bzero(&addr, sizeof addr);
  socklen_t addrlen = static_cast<socklen_t>(sizeof addr);
  void* any = &addr;
  if (::getsockname(sockfd, 
            static_cast<struct sockaddr*>(any), &addrlen) < 0) {
    LOG_FATAL("TcpClient::newConnection() getsockname");
  }
  // FIXME poll with zero timeout to double confirm the new connection
  // FIXME use make_shared if necessary
  TcpConnectionPtr conn(new TcpConnection(loop_, sockfd, InetAddress(addr),
                                          peerAddr, std::string(buf)));

  conn->setConnectionCallback(connectionCallback_);
  conn->setMessageCallback(messageCallback_);
  conn->setWriteCompleteCallback(writeCompleteCallback_);
  conn->setCloseCallback(
      std::bind(&TcpClient::closeConnection, this, _1)); // FIXME: unsafe
  
  connection_ = conn;  // FIXEME : thread safe
  conn->connectEstablished();
}

void TcpClient::closeConnection(const TcpConnectionPtr& conn) {
  loop_->assertInLoopThread();
  connection_.reset(); // FIXME :thread safe

  loop_->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
  LOG_WARN("the server close the connect");
}

