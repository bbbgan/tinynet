#include <example/webserver/HttpContext.h>
#include <tinynet/net/Buffer.h>
#include <regex>
#include <tinynet/base/Logger.h>

using namespace tinynet;
using namespace tinynet::net;
using namespace std;

void HttpContext::parseRequest(Buffer* buf, Timestamp recvTime) {
  bool hasMore = true;
  while (hasMore) {
    const char* crlf = buf->findCRLF();
    if (crlf == nullptr) return;
    std::string line(buf->peek(), crlf);
    buf->retrieveUntil(crlf + 2);
    switch (state_) {
      case REQUEST_LINE:
        if (!parseRequestLine(line)) {
          request_.setIsBadRequest(true);
          return;
        }
        break;
      case HEADERS:
        parseHeader(line);
        break;
      case BODY:
        parseBody(line);
        break;
      default:
        break;
    }
  }
}

bool HttpContext::parseRequestLine(const string& line) {
  regex patten("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
  smatch subMatch;
    if(regex_match(line, subMatch, patten)) {  
        request_.setMethod(subMatch[1]);
        request_.setPath(subMatch[2]);
        if (subMatch[3] == "1.0")
          request_.setVersion(HttpRequest::kHttp10);
        else if ((subMatch[3] == "1.1"))
          request_.setVersion(HttpRequest::kHttp11);
        else
          request_.setIsBadRequest(true);
        state_ = HEADERS;
        return true;
    }
    LOG_ERROR("RequestLine Error");
    return false;  
}
void HttpContext::parseHeader(const string& line) {
  regex patten("^([^:]*): ?(.*)$");
  smatch subMatch;
  if (regex_match(line, subMatch, patten)) {
    request_.addHeader(subMatch[1], subMatch[2]);
  } else {
    state_ = BODY;
  }
}
void HttpContext::parseBody(const string& line) {
  state_ = FINISH;
}

