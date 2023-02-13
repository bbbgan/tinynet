// 
// Author       : gan
// Date         : 2022-08
// 
#ifndef TINYNET_NET_SOCKET_H_
#define TINYNET_NET_SOCKET_H_

#include <tinynet/base/noncopyable.h>

struct tcp_info;
namespace tinynet 
{
namespace net
{
class InetAddress;
// Take over the sockfd and responsible for close
class Socket : noncopyable {
 public:
  explicit Socket(int sockfd) : sockfd_(sockfd) {}
  ~Socket();

  int fd() const { return sockfd_; }
  void bindAddress(const InetAddress& localAddr);
  void listen();
  int accept(InetAddress* peeraddr);
  void shutdownWrite();

  void setTcpNoDelay(bool on);
  void setReuseAddr(bool on);
  void setReusePort(bool on);
  void setKeepAlive(bool on);

 private:
  const int sockfd_;
};

}  // namespace net

}  // namespace tinynet

#endif  // TINYNET_NET_SOCKET_H_