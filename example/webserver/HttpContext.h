#ifndef TINYNET_NET_HTTPCONTEXT_H_
#define TINYNET_NET_HTTPCONTEXT_H_

#include <tinynet/net/Buffer.h>
#include <tinynet/base/Timestamp.h>
#include <example/webserver/HttpRequest.h>

namespace tinynet
{
namespace net
{
class HttpContext {
 public:
  enum ParaseState {
    REQUEST_LINE,
    HEADERS,
    BODY,
    FINISH,      
  };

  HttpContext() : state_(REQUEST_LINE) {}

  void parseRequest(Buffer* buf, Timestamp recvTime);
  bool isFinish() const { return state_ == FINISH; }
  const HttpRequest& request() const {
    return request_;
  }
  HttpRequest& request() {
    return request_;
  }
  void reset() {
    state_ = REQUEST_LINE;
    HttpRequest dummy;
    request_.swap(dummy);
  }
 private:
  bool parseRequestLine(const std::string& line);
  void parseHeader(const std::string& line);
  void parseBody (const std::string& line);

  // FIXME: parsePost
  
  ParaseState state_;
  HttpRequest request_;
};

} // namespace net
} // namespace tinynet


#endif // TINYNET_NET_HTTPCONTEXT_H_