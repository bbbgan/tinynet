// 
// Author       : gan
// Date         : 2022-08
// 
#include <tinynet/net/InetAddress.h>
#include <tinynet/base/Logger.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <strings.h>

using namespace tinynet;
using namespace tinynet::net;

InetAddress::InetAddress(uint16_t port, bool loopback) {
  bzero(&addr_, sizeof addr_);
  addr_.sin_family = AF_INET;
  in_addr_t ip = loopback ? INADDR_LOOPBACK : INADDR_ANY;
  addr_.sin_addr.s_addr = ::htonl(ip);
  addr_.sin_port = ::htons(port);
}

InetAddress::InetAddress(const std::string& ip, uint16_t port) {
  bzero(&addr_, sizeof addr_);
  addr_.sin_family = AF_INET;
  addr_.sin_port = ::htons(port);
  if (::inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr.s_addr) <= 0) {
    LOG_FATAL("InetAddress::inet_pton()");
  }
}

std::string InetAddress::toIp() const {
  char buf[INET_ADDRSTRLEN];
  const char* ret = inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
  if (ret == nullptr) {
    buf[0] = '\0';
    LOG_ERROR("InetAddress::inet_ntop()");
  }
  return std::string(buf);
}

uint16_t InetAddress::port() const { return ntohs(addr_.sin_port); }

std::string InetAddress::toIpPort() const {
  std::string res = toIp();
  res.push_back(':');
  return res.append(std::to_string(port()));
}