// 
// Author       : gan
// Date         : 2022-09-05
//
#ifndef TINYNET_NET_TCPCONNECTION_H_
#define TINYNET_NET_TCPCONNECTION_H_

#include <memory>

#include <tinynet/base/noncopyable.h>
#include <tinynet/net/Channel.h>
#include <tinynet/net/InetAddress.h>
#include <tinynet/net/Socket.h>
#include <tinynet/net/Callbacks.h>
#include <tinynet/net/Buffer.h>

struct tcp_info;

namespace tinynet 
{
namespace net 
{
class EventLoop;
class TcpConnection : noncopyable,
                      public std::enable_shared_from_this<TcpConnection> 
{
 public:
  TcpConnection(EventLoop* loop, int sockfd,
                const InetAddress& localAddr, 
                const InetAddress& peerAddr,
                const std::string& name);
  ~TcpConnection();

  EventLoop* getLoop() const { return loop_; }
  const InetAddress& localAddress() const { return localAddr_; }
  const InetAddress& peerAddress() const { return peerAddr_; }
  const std::string& name() const { return name_; }
  bool connected() const { return state_ == kConnected; }
  bool disconnected() const { return state_ == kDisconnected; }

  void send(const void* message, int len); 
  // void send(std::string_view data); FIXME C++14
  // this one will swap data
  void send(Buffer* message); 
  void send(const std::string& str);
  // NOT thread safe, no simultaneous calling
  void shutdown();            
  void forceClose();
  void setTcpNoDelay(bool on);

  void startRead();
  void stopRead();

  void setConnectionCallback(const ConnectionCallback& cb) {
    connectionCallback_ = cb;
  }
  void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
  void setWriteCompleteCallback(const WriteCompleteCallback& cb) {
    writeCompleteCallback_ = cb;
  }
  void setHighWaterMarkCallback(const HighWaterMarkCallback& cb,
                                size_t highWaterMark) {
    highWaterMarkCallback_ = cb;
    highWaterMark_ = highWaterMark;
  }

  Buffer* inputBuffer() { return &inputBuffer_; }
  Buffer* outputBuffer() { return &outputBuffer_; }

  /// Internal use only.
  void setCloseCallback(const CloseCallback& cb) { closeCallback_ = cb; }
  // called when TcpServer accepts a new connection
  void connectEstablished();  
  void connectDestroyed();  

 private:                             
  void handleRead(Timestamp receiveTime); 
  void handleWrite();
  void handleClose();
  void handleError();

  void sendInLoop(const void* message, size_t len);
  void sendInLoop(const std::string& message);
  void shutdownInLoop();
  void forceCloseInLoop();

  int stateAtomicGetAndSet(int newState);
  enum ConnectionState { kConnecting, kConnected, kDisconnecting, kDisconnected };

  EventLoop* loop_;  
  int state_;  
  std::unique_ptr<Socket> socket_;  
  std::unique_ptr<Channel> channel_;  
  const InetAddress localAddr_;
  const InetAddress peerAddr_;
  size_t highWaterMark_; 
  std::string name_;
  Buffer inputBuffer_;                          
  Buffer outputBuffer_;  // FIXME: use list<Buffer> as output buffer.        
  ConnectionCallback connectionCallback_;       
  MessageCallback messageCallback_;             
  WriteCompleteCallback writeCompleteCallback_;  
  HighWaterMarkCallback highWaterMarkCallback_;  
  CloseCallback closeCallback_;                                     
};

}  // namespace net

}  // namespace tinynet

#endif  // TINYNET_NET_TCPCONNECTION_H_