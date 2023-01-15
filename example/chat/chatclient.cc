// 
// Author       : gan
// Date         : 2022-09-12
//
#include <tinynet/net/TcpClient.h>
#include <tinynet/net/EventLoopThread.h>
#include<tinynet/base/Logger.h>

#include <example/chat/codec.h>

#include <stdio.h>
#include <unistd.h>
#include <iostream>

using namespace tinynet;
using namespace tinynet::net;

class ChatClient : noncopyable 
{
 public:
  ChatClient(EventLoop* loop, const InetAddress& peerAddr)
    : client_(loop, peerAddr),
      codec_(std::bind(&ChatClient::onMessageComplete, this, _1, _2, _3))
  {
    client_.setConnectionCallback(
        std::bind(&ChatClient::onConnection, this, _1));
    client_.setMessageCallback(
        std::bind(&LengthHeaderCodec::decodeMessage, &codec_, _1, _2, _3));
  }
  void connect() {
    client_.startconnect();
  }
  void disconnect() {
    client_.disconnect();
  }
  void write(const std::string msg) {
    std::lock_guard<std::mutex> lk(mutex_);
    if (connection_) {
      codec_.encodeMessage(connection_.get(), msg);
    }
  }
 private:
  void onConnection(const TcpConnectionPtr& conn) {
    LOG_INFO("connection %s -> %s %s",
            conn->peerAddress().toIpPort().c_str(),
            conn->localAddress().toIpPort().c_str(),
            conn->connected() ? "up" : "down");
    std::lock_guard<std::mutex> lk(mutex_);
    if (conn->connected()) {
      connection_ = conn;
    } else {
      connection_.reset();
    }
  }
  void onMessageComplete(const TcpConnectionPtr&,
                         const std::string& message, Timestamp stamp)
  {
    printf("<<<< %s\n", message.c_str());
  }

  TcpClient client_;
  LengthHeaderCodec codec_;
  std::mutex mutex_;
  TcpConnectionPtr connection_;
};

// client cmd : tinynet/tinynet-build/Debug/bin/chat_client 127.0.0.1 9999
int main(int argc, char* argv[])
{
  Logger::setLogLevel(Logger::LogLevel::INFO);
  LOG_INFO("main thread: pid = %d, tid = %d", getpid(), gettid());
  if (argc > 2)
  {
    EventLoopThread loopThread;
    uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
    InetAddress serverAddr(argv[1], port);

    ChatClient client(loopThread.startLoop(), serverAddr);
    client.connect();
    std::string line;
    while (std::getline(std::cin, line))
    {
      client.write(line);
    }
    client.disconnect();
    sleep(1000);
    // wait for disconnect, it will not shutdown immediately for waiting for write completely
  }
  else
  {
    printf("Usage: %s host_ip port\n", argv[0]);
  }
}
