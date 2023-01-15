#include <tinynet/net/TcpServer.h>
#include <tinynet/net/EventLoop.h>
#include <tinynet/net/InetAddress.h>
#include <tinynet/base/Logger.h>

#include <utility>

#include <stdio.h>
#include <unistd.h>

using namespace tinynet;
using namespace tinynet::net;


class EchoServer {
 public: 
  EchoServer(EventLoop* loop, const InetAddress& localAddr) 
    : baseLoop_(loop), server_(new TcpServer(loop, localAddr, "EchoServer")) {
        server_->setConnectionCallback(std::bind(
                &EchoServer::onConnection, this, _1));
        server_->setMessageCallback(std::bind(
                &EchoServer::onMessage, this, _1, _2, _3));
        server_->setWriteCompleteCallback(std::bind(
                &EchoServer::onWriteComplete, this, _1));
    }
  ~EchoServer();  
  void start() {
    server_->start();
  }
  void setThreadNum(int n) {
    server_->setThreadNum(n);
  }
 private:
  void onConnection(const TcpConnectionPtr& conn) {
    LOG_INFO("connection [%s] is %s",
              conn->name().c_str(),
              conn->connected() ? "up":"down");
    conn->send(std::string("hello"));
  }
  void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp stamp) {
    LOG_INFO("connection [%s] receive %lubytes",
              conn->name().c_str(),
              buf->readableBytes());
    conn->send(buf);
  }
  void onWriteComplete(const TcpConnectionPtr& conn) {
    LOG_INFO("connection [%s] write to [%s] complete", conn->name().c_str(),
             conn->peerAddress().toIpPort().c_str());
  }
  
  EventLoop* baseLoop_;
  std::unique_ptr<TcpServer> server_;

};
EchoServer::~EchoServer() = default;

// tinynet/tinynet-build/Debug/bin/EchoServer_test 9999 1
// client cmd : nc -nv 127.0.0.1 9999
int main(int argc, char* argv[]) {
  Logger::setLogLevel(Logger::LogLevel::TRACE);
  if (argc > 1) {
    LOG_INFO("main thread: pid = %d, tid = %d", getpid(), gettid());
    EventLoop loop;
    InetAddress localAddr(static_cast<uint16_t>(atoi(argv[1])), false);
    EchoServer server(&loop, localAddr);
    if (argc > 2)
    {
      server.setThreadNum(atoi(argv[2]));
    }
    server.start();
    loop.loop();
  } else {
    printf("Usage: %s port [thread_num]\n", argv[0]);
  }
  return 0;
}


