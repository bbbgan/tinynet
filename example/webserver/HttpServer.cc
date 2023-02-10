#include <example/webserver/HttpServer.h>
#include <example/webserver/HttpContext.h>
#include <example/webserver/HttpResponse.h>
#include <tinynet/base/Logger.h>

using namespace tinynet;
using namespace tinynet::net;

HttpServer::HttpServer(EventLoop* loop, const InetAddress& listenAddr) 
                      : server_(loop, listenAddr, "Tiny WebServer") 
{
  server_.setConnectionCallback(
      std::bind(&HttpServer::onConnection, this, _1));
  server_.setMessageCallback(
      std::bind(&HttpServer::onMessage, this, _1, _2, _3));     
}

void HttpServer::onConnection(const TcpConnectionPtr& conn) {
  if (conn->connected()) {
    conn->setContext(HttpContext());
  }
}

void HttpServer::onMessage(const TcpConnectionPtr& conn, Buffer* buf,
                           Timestamp receiveTime) {
  HttpContext* context =
      std::any_cast<HttpContext>(conn->getMutableContext());
  context->parseRequest(buf, receiveTime);
  if (context->isFinish()) {
    onRequest(conn, context->request());
    context->reset();
  }
}

void HttpServer::onRequest(const TcpConnectionPtr& conn,
                           const HttpRequest& req) {
  const std::string& connection = req.getHeader("Connection");
  bool close =
      connection == "close" ||
      (req.getVersion() == HttpRequest::kHttp10 && connection != "Keep-Alive");
  HttpResponse response(close);
  httpCallback_(req, &response); 
  conn->send(response.getBuffer());
  if (response.closeConnection()) {
    conn->shutdown();
  }
}
