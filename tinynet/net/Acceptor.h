// 
// Author       : gan
// Date         : 2022-09
// 
#ifndef TINYNET_NET_ACCEPTOR_H_
#define TINYNET_NET_ACCEPTOR_H_

#include <functional>

#include <tinynet/net/Channel.h>
#include <tinynet/net/Socket.h>

namespace tinynet 
{
namespace net 
{
class EventLoop;
class InetAddress;

// accept the tcp 
class Acceptor : noncopyable {
 public:
  typedef std::function<void(int sockfd, const InetAddress &)> NewConnectionCallback;
  Acceptor(EventLoop *loop, const InetAddress& listenAddr, bool reusePort);
  ~Acceptor();

  void setNewConnectionCallback(const NewConnectionCallback& cb) {
    newConnectionCallback_ = cb;
  }
  void listen();

 private:
  void handleRead();

  EventLoop *loop_;
  InetAddress local_;
  Socket acceptSocket_;
  Channel acceptChannel_;
  NewConnectionCallback newConnectionCallback_; 
  int idleFd_;
};
}  // namespace net
}  // namespace tinynet

#endif  // TINYNET_NET_ACCEPTOR_H_