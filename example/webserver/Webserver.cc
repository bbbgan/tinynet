#include <example/webserver/HttpContext.h>
#include <example/webserver/HttpRequest.h>
#include <example/webserver/HttpResponse.h>
#include <example/webserver/HttpServer.h>
#include <tinynet/net/TcpServer.h>
#include <tinynet/net/EventLoop.h>
#include<tinynet/base/Logger.h>

using namespace tinynet;
using namespace tinynet::net;
// FIXME 
// conn的零拷贝技术
// 可以在writecompletely里面unmmap

void onRequest(const HttpRequest& req, HttpResponse* resp) {
  resp->setPath(req.path());
  if (req.isBadRequest()) {
    resp->setStatusCode(HttpResponse::k400BadRequest);
  }
  resp->fillBuf();
}

int main(int argc, char* argv[]) {
  Logger::setLogLevel(Logger::LogLevel::INFO);
  EventLoop loop;
  HttpServer server(&loop, InetAddress(9999));
  server.setHttpCallback(onRequest);
  server.setThreadNum(0);
  server.start();
  loop.loop();
}




  // const char CRLF[] = "\r\n";
  // while (buf->readableBytes() && state_ != FINISH) {
  //   const char* lineEnd = search(buf->peek(), buf->beginWrite(), CRLF, CRLF + 2);
  //   std::string line(buf.Peek(), lineEnd);
  //   switch (state_) {
  //     case REQUEST_LINE:
  //       if (!parseRequestLine(line)) {
  //         request_.setIsBadRequest(true);
  //         return false;
  //       }
  //       parsePath(); break;
  //     case HEADERS:
  //       parseHeader(line);
  //       // only "/r/n" remain in the buf
  //       if (buf->readableBytes() <= 2) 
  //         state_ = FINISH;
  //       break;
  //     case BODY:
  //       parseBody(line);
  //       break;
  //     default:
  //       break;
  //   }
  //   if (lineEnd == buf->beginWrite()) { buf->retrieveAll(); break; }
  //   buf->retrieveUntil(lineEnd + 2);
  // }
  // // LOG_DEBUG() request.dump();
  // request_.setIsBadRequest(false);