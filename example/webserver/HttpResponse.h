#ifndef TINYNET_NET_HTTPRESPONSE_H_
#define TINYNET_NET_HTTPRESPONSE_H_

#include <map>
#include <tinynet/net/Buffer.h>
#include <string>
#include <sys/stat.h>    // stat

namespace tinynet
{
namespace net
{

class HttpResponse {
 public:
  enum HttpStatusCode {
    kUnknown,
    k200Ok = 200,
    k400BadRequest = 400,
    k403Forbidden = 403,
    k404NotFound = 404,
  };
  explicit HttpResponse(bool close)
    : statusCode_(kUnknown),
      closeConnection_(close) {}

  void setStatusCode(HttpStatusCode code)
  { statusCode_ = code; }

  void setContentType(const std::string& contentType)
  { addHeader("Content-Type", contentType); }

  void addHeader(const std::string& key, const std::string& value)
  { headers_[key] = value; }

  void setPath(const std::string& path)
  { path_ = path; }

  void setSrcDir(const std::string& path) 
  { srcDir_ = path; }
  
  const std::string& getSrcDir() const 
  {  return srcDir_; }
  bool closeConnection() const 
  { return closeConnection_; }
  Buffer* getBuffer() {
    return &buf;
  }
  void fillBuf();
  
 private:
  void setErrorHtml();
  void addStateLineAppendBuf();
  void addHeaderAppendBuf();
  void addContentAppendBuf();
  void errorContent(std::string&&);

  std::map<std::string, std::string> headers_;
  HttpStatusCode statusCode_;
  bool closeConnection_;
  std::string body_;

  char* mmFile_; 
  struct stat mmFileStat_;
  std::string path_;
  static std::string srcDir_;
  Buffer buf;
};

} // namespace net


} // namespace tinynet


#endif // TINYNET_NET_HTTPRESPONSE_H_