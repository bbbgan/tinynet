// 
// Author       : gan
// Date         : 2022-09
//
#ifndef TINYNET_NET_TCPSERVER_H_
#define TINYNET_NET_TCPSERVER_H_

#include <tinynet/net/TcpConnection.h>

#include <atomic>
#include <map>

namespace tinynet
{
namespace net
{
class Acceptor;
class EventLoop;
class EventLoopThreadPool;

class TcpServer : noncopyable
{
 public:
  typedef std::function<void(EventLoop*)> ThreadInitCallback;
  TcpServer(EventLoop* loop, const InetAddress& local, 
            const std::string& name, bool reuseport = false);
  ~TcpServer();  // force out-line dtor, for std::unique_ptr members.

  const std::string& ipPort() const { return ipPort_; }
  const std::string& name() const { return name_; }
  EventLoop* getLoop() const { return loop_; }

  void setThreadNum(int numThreads);
  void setThreadInitCallback(const ThreadInitCallback& cb)
  { threadInitCallback_ = cb; }
  std::shared_ptr<EventLoopThreadPool> threadPool()
  { return threadPool_; }
  // Thread safe.
  void start();
  void setConnectionCallback(const ConnectionCallback& cb)
  { connectionCallback_ = cb; }
  void setMessageCallback(const MessageCallback& cb)
  { messageCallback_ = cb; }
  void setWriteCompleteCallback(const WriteCompleteCallback& cb)
  { writeCompleteCallback_ = cb; }

 private:
  /// Not thread safe, but in loop
  void newConnection(int sockfd, const InetAddress& peerAddr);
  /// Thread safe.
  void removeConnection(const TcpConnectionPtr& conn);
  /// Not thread safe, but in loop
  void removeConnectionInLoop(const TcpConnectionPtr& conn);
  typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;

  EventLoop* loop_;  // the acceptor loop
  InetAddress localAddr_;
  const std::string ipPort_;
  const std::string name_;
  std::unique_ptr<Acceptor> acceptor_; 
  std::shared_ptr<EventLoopThreadPool> threadPool_;
  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  WriteCompleteCallback writeCompleteCallback_;
  ThreadInitCallback threadInitCallback_;
  std::atomic_bool started_;
  int nextConnId_;
  ConnectionMap connMap_;
};
} // namespace net
} // namespace tinynet


#endif // TINYNET_NET_TCPSERVER_H_