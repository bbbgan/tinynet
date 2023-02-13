// 
// Author       : gan
// Date         : 2022-09
//
#ifndef TINYNET_NET_TCPCLIENT_H_
#define TINYNET_NET_TCPCLIENT_H_

#include <tinynet/base/noncopyable.h>
#include <tinynet/net/TcpConnection.h>


namespace tinynet
{
namespace net
{
class Connector;
// this class is not thread safe
class TcpClient : noncopyable
{
 public:
  TcpClient(EventLoop* loop, const InetAddress& peerAddr);
  ~TcpClient();  

  void startconnect();
  // The func disconnect will not take effect immediately
  // Waiting for the  write completely
  void disconnect();

  EventLoop* getLoop() const { return loop_; }

  void setConnectionCallback(ConnectionCallback cb)
  { connectionCallback_ = std::move(cb); }
  void setMessageCallback(MessageCallback cb)
  { messageCallback_ = std::move(cb); }
  void setWriteCompleteCallback(WriteCompleteCallback cb)
  { writeCompleteCallback_ = std::move(cb); }
 private:
  void newConnection(int sockfd, const InetAddress& peerAddr);
  void closeConnection(const TcpConnectionPtr& conn);
 private:
  typedef std::shared_ptr<Connector> ConnectorPtr; // FIXME: unique_ptr ?
  EventLoop* loop_;
  ConnectorPtr connector_;
  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  WriteCompleteCallback writeCompleteCallback_;
  bool startconnect_; // atomic
  TcpConnectionPtr connection_;
};
} // namespace net
} // namespace tinynet



#endif // TINYNET_NET_TCPCLIENT_H_