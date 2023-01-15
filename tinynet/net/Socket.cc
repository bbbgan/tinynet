// 
// Author       : gan
// Date         : 2022-08-29
// 
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>  
#include <sys/socket.h>
#include <sys/uio.h>  
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <cassert>
#include <string.h> 

#include <tinynet/net/InetAddress.h>
#include <tinynet/net/Socket.h>
#include <tinynet/base/Logger.h>

using namespace tinynet;
using namespace tinynet::net;

namespace
{
int socketAccept(int sockfd, struct sockaddr_in* addr) {
  socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);
  void* anyAddr = addr;
  int connfd = ::accept4(sockfd, static_cast<struct sockaddr*>(anyAddr),
                          &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
  if (connfd < 0) {
    int savedErrno = errno;
    LOG_ERROR("Socket::socketAccept()");
    switch (savedErrno) {
      case EAGAIN:
      case ECONNABORTED:
      case EINTR:
      case EPROTO:  
      case EPERM:
      case EMFILE: 
        errno = savedErrno;
        break;
      case EBADF:
      case EFAULT:
      case EINVAL:
      case ENFILE:
      case ENOBUFS:
      case ENOMEM:
      case ENOTSOCK:
      case EOPNOTSUPP:
        // unexpected errors
        LOG_FATAL( "unexpected error of ::accept %d", savedErrno);
        break;
      default:
        LOG_FATAL( "unexpected error of ::accept %d", savedErrno);
        break;
    }
  }
  return connfd;
}
}

Socket::~Socket() {
  ::close(sockfd_);
}

void Socket::bindAddress(const InetAddress& localAddr) {
  int ret = ::bind(sockfd_, localAddr.getSockAddr(), static_cast<socklen_t>(sizeof(struct sockaddr_in)));
  if (ret < 0)
    LOG_FATAL("Socket::bindAddress():errno");
  else
    LOG_TRACE("bind to addr[%s] success", localAddr.toIpPort().c_str());
}

void Socket::listen() {
  int ret = ::listen(sockfd_, SOMAXCONN);  
  LOG_TRACE("listen success %d", ret); // FIXME :REMOVE
  if (ret < 0)
    LOG_FATAL("Socket::listen():errno");
}

int Socket::accept(InetAddress* peeraddr) {
  struct sockaddr_in addr;
  bzero(&addr, sizeof addr);
  int connfd = socketAccept(sockfd_, &addr);
  if (connfd >= 0) {
    peeraddr->setSockAddrInet(addr);
  }
  return connfd;
}

void Socket::shutdownWrite() {
  if (::shutdown(sockfd_, SHUT_WR) < 0) {
    LOG_ERROR("Socket::shutdownWrite()");
  }
}

void Socket::setTcpNoDelay(bool on) {
  int optval = on ? 1 : 0;
  int ret = ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval,
               static_cast<socklen_t>(sizeof optval));
  if (ret < 0)
    LOG_FATAL("Socket::setTcpNoDelay()");
}

void Socket::setReuseAddr(bool on) {
  int optval = on ? 1 : 0;
  int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval,
               static_cast<socklen_t>(sizeof optval));
  if (ret < 0)
    LOG_FATAL("Socket::setReuseAddr()");
}

void Socket::setReusePort(bool on) {
  int optval = on ? 1 : 0;
  int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval,
                         static_cast<socklen_t>(sizeof optval));
  if (ret < 0)
    LOG_FATAL("Socket::setReusePort()");
}

void Socket::setKeepAlive(bool on) {
  int optval = on ? 1 : 0;
  int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval,
               static_cast<socklen_t>(sizeof optval));
  if (ret < 0)
  LOG_FATAL("Socket::setKeepAlive()");
}
