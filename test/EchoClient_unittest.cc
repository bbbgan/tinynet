#include <tinynet/base/Logger.h>
#include <tinynet/net/TcpClient.h>
#include <tinynet/net/InetAddress.h>
#include <tinynet/net/EventLoop.h>

#include <utility>

#include <stdio.h>
#include <unistd.h>

using namespace tinynet;
using namespace tinynet::net;

class EchoClient {
 public:
  EchoClient(EventLoop* loop, const InetAddress& listenAddr)
    : loop_(loop),
      client_(loop, listenAddr)
  {
    client_.setConnectionCallback(
        std::bind(&EchoClient::onConnection, this, _1));
    client_.setMessageCallback(
        std::bind(&EchoClient::onMessage, this, _1, _2, _3));
  }
  void connect() { client_.startconnect(); }

 private:
  void onConnection(const TcpConnectionPtr& conn) {
    LOG_INFO("connection %s -> %s %s",
            conn->peerAddress().toIpPort().c_str(),
            conn->localAddress().toIpPort().c_str(),
            conn->connected() ? "up" : "down");
    conn->send("world\n");
  }
  void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time) {
    std::string msg(buf->retrieveAllAsString());
    LOG_INFO("connection [%s] receive %lubytes", conn->name().c_str(),
             buf->readableBytes());
    if (msg == "quit\n") {
      conn->send("bye\n");
      conn->shutdown();
    } else if (msg == "shutdown\n") {
      loop_->quit();
    } else {
      conn->send(msg);
    }
  }
 private:
  EventLoop* loop_;
  TcpClient client_;
};

// server cmd: nc -lp 9999
// tinynet/tinynet-build/Debug/bin/EchoClient_test 127.0.0.1 9999
int main(int argc, char* argv[]) {
  Logger::setLogLevel(Logger::LogLevel::INFO);
  LOG_INFO("main thread: pid = %d, tid = %d", getpid(), gettid());
  if (argc > 2) {
    EventLoop loop; 
    InetAddress serverAddr(argv[1], static_cast<uint16_t>(atoi(argv[2])));
    EchoClient echoClient(&loop, serverAddr);
    echoClient.connect();
    loop.loop();
  } else {
    printf("Usage: %s ip port \n", argv[0]);
  }
}