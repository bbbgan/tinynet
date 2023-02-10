#ifndef TINYNET_NET_HTTPSERVER_H_
#define TINYNET_NET_HTTPSERVER_H_

#include <tinynet/net/TcpServer.h>
#include <example/webserver/HttpRequest.h>
#include <example/webserver/HttpResponse.h>

namespace tinynet
{
namespace net
{
class HttpServer {
 public:
  typedef std::function<void (const HttpRequest&,
                              HttpResponse*)> HttpCallback;
  HttpServer(EventLoop* loop,
             const InetAddress& listenAddr);
  
  void setHttpCallback(const HttpCallback& cb) {
    httpCallback_ = cb; 
  }
  void setThreadNum(int num) {
    server_.setThreadNum(num);
  }
  void start() {
    server_.start();
  }
 private:
  void onConnection(const TcpConnectionPtr& conn);
  void onMessage(const TcpConnectionPtr& conn,
                 Buffer* buf,
                 Timestamp receiveTime);
  void onRequest(const TcpConnectionPtr&, const HttpRequest&);
  TcpServer server_;
  HttpCallback httpCallback_;
};

} // namespace net
} // namespace tinynet



#endif // TINYNET_NET_HTTPSERVER_H_