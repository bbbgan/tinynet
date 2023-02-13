// 
// Author       : gan
// Date         : 2022-08
// 
#ifndef TINYNET_INET_INETADDRESS_H_
#define TINYNET_INET_INETADDRESS_H_

#include <string>
#include <netinet/in.h>

namespace tinynet 
{
namespace net 
{
// Wrapper of sockaddr_in.
class InetAddress {
 public:
  // Mostly used in TcpServer listening.
  explicit InetAddress(uint16_t port = 0, bool loopback = false);
  /// ip should be "1.2.3.4"
  InetAddress(const std::string& ip, uint16_t port);
  explicit InetAddress(const struct sockaddr_in& addr) : addr_(addr) {}
  
  sa_family_t family() const { return addr_.sin_family; }
  socklen_t getSocklen() const { return sizeof(addr_); }
  void setSockAddrInet(const struct sockaddr_in& addr) { addr_ = addr; }
  const struct sockaddr* getSockAddr() const {
    return reinterpret_cast<const struct sockaddr*>(&addr_);
  }
  
  std::string toIp() const;
  uint16_t port() const;
  std::string toIpPort() const;
 private:
  struct sockaddr_in addr_;
};

}  // namespace net
}  // namespace tinynet

#endif  // TINYNET_INET_INETADDRESS_H_