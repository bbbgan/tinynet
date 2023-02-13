// 
// Author       : gan
// Date         : 2022-09
//
#include <tinynet/net/TcpServer.h>
#include <tinynet/net/EventLoop.h>
#include<tinynet/base/Logger.h>

#include <example/chat/codec.h>

#include <set>
#include <stdio.h>
#include <unistd.h>

using namespace tinynet;
using namespace tinynet::net;

class ChatServer : noncopyable
{
 public:
  ChatServer(EventLoop* loop, const InetAddress& listenAddr) 
  : server_(loop, listenAddr, "ChatServer"),
    codec_(std::bind(&ChatServer::onMessageComplete, this, _1, _2, _3)),
    connections_(new ConnectionList)
  {
    server_.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));
    server_.setMessageCallback(std::bind(&LengthHeaderCodec::decodeMessage, &codec_, _1, _2, _3));
  }
  void setThreadNum(int num) {
    server_.setThreadNum(num);
  }
  void start() {
    server_.start();
  }
 private:
  typedef std::set<TcpConnectionPtr> ConnectionList;
  typedef std::shared_ptr<ConnectionList> ConnectionListPtr;

  void onConnection(const TcpConnectionPtr& conn) {
    LOG_INFO("connection %s -> %s %s",
            conn->peerAddress().toIpPort().c_str(),
            conn->localAddress().toIpPort().c_str(),
            conn->connected() ? "up" : "down");
    std::lock_guard<std::mutex> lk(mutex_);
    if (!connections_.unique()) {
      connections_.reset(new ConnectionList(*connections_));
    }
    assert(connections_.unique());
    if (conn->connected()) {
      connections_->insert(conn);
    } else {
      connections_->erase(conn);
    }
  }
  ConnectionListPtr getConnectionList() {
    std::lock_guard<std::mutex> lk(mutex_);
    return connections_;
  }
  void onMessageComplete(const TcpConnectionPtr&,
                         const std::string& message, Timestamp stamp)
  {
    ConnectionListPtr connections = getConnectionList();
    for(auto it = connections->begin();
        it != connections->end(); ++it) {
      codec_.encodeMessage(it->get(), message);
    }
  }
  TcpServer server_;
  LengthHeaderCodec codec_;
  std::mutex mutex_;
  ConnectionListPtr connections_;
};
// server cmd : tinynet/tinynet-build/Debug/bin/chat_server 9999 2
int main(int argc, char* argv[]) {
  Logger::setLogLevel(Logger::LogLevel::INFO);
  LOG_INFO("main thread: pid = %d, tid = %d", getpid(), gettid());
  if (argc > 1) {
    EventLoop loop;
    uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
    InetAddress serverAddr(port);
    ChatServer server(&loop, serverAddr);
    if (argc > 2) {
      server.setThreadNum(atoi(argv[2]));
    }
    server.start();
    loop.loop();
  } else {
    printf("Usage: %s port [thread_num]\n", argv[0]);
  }
  return 0;
}