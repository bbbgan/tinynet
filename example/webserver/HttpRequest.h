#ifndef TINYNET_NET_HTTPREQUEST_H_
#define TINYNET_NET_HTTPREQUEST_H_

#include <string>
#include <map>
#include <stdio.h>
#include <assert.h>

#include <tinynet/net/Buffer.h>

namespace tinynet
{
namespace net
{
class HttpRequest {
 public:
  enum Version
  {
    kUnknown, kHttp10, kHttp11
  };
	void setVersion(Version v) {
		version_ = v;
	}

  Version getVersion() const
  { return version_; }
	void setMethod(const std::string& method) {
		method_ = method;
	}
  const std::string& method() const
  { return method_; }	
  void setPath(const std::string& path)
  {
    path_ = path;
  }
  const std::string& path() const
  { return path_; }
	void addHeader(const std::string& key, const std::string& val) {
		headers_[key] = val;
	}
  std::string getHeader(const std::string& field) const
  {
   std::string result;
    auto it = headers_.find(field);
    if (it != headers_.end())
    {
      result = it->second;
    }
    return result;
  }
  const std::map<std::string, std::string>& headers() const
  { return headers_; }
  // FIXME: use uniqueptr for this class
  void swap(HttpRequest& that)
  {
    std::swap(method_, that.method_);
    std::swap(version_, that.version_);
    path_.swap(that.path_);
    headers_.swap(that.headers_);
  }
  bool isBadRequest() const {
    return isBadRequest_;
  }
  void setIsBadRequest(bool f) {
    isBadRequest_ = f;
  }
 private:
  Version version_;
  std::string method_, path_, body_;
  bool isBadRequest_;
  std::map<std::string, std::string> headers_;
  std::map<std::string, std::string> post_;
};

} // namespace net


} // namespace tinynet


#endif // TINYNET_NET_HTTPREQUEST_H_