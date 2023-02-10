#include <example/webserver/HttpResponse.h>
#include <tinynet/base/Logger.h>
#include <tinynet/net/Buffer.h>

#include <fcntl.h>       // open
#include <unistd.h>      // close
#include <sys/stat.h>    // stat
#include <sys/mman.h>    // mmap, munmap
#include <unordered_map>

using namespace tinynet;
using namespace tinynet::net;
using namespace std;

static  unordered_map<string, string> SUFFIX_TYPE = {
    {".html", "text/html"},
    {".xml", "text/xml"},
    {".xhtml", "application/xhtml+xml"},
    {".txt", "text/plain"},
    {".rtf", "application/rtf"},
    {".pdf", "application/pdf"},
    {".word", "application/nsword"},
    {".png", "image/png"},
    {".gif", "image/gif"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".au", "audio/basic"},
    {".mpeg", "video/mpeg"},
    {".mpg", "video/mpeg"},
    {".avi", "video/x-msvideo"},
    {".gz", "application/x-gzip"},
    {".tar", "application/x-tar"},
    {".css", "text/css "},
    {".js", "text/javascript "},
};
static  unordered_map<int, string> CODE_STATUS = {
    {200, "OK"},
    {400, "Bad Request"},
    {403, "Forbidden"},
    {404, "Not Found"},
};
static  unordered_map<int, string> CODE_PATH = {
    {400, "/400.html"},
    {403, "/403.html"},
    {404, "/404.html"},
};
static const string plainStr("text/plain");

namespace
{
// the return val is the reference of static val, not local val
const string& getFileType(const string& path) {
  string::size_type idx = path.find_last_of('.');
  if (idx == string::npos) {
    return plainStr;
  }
  string suffix = path.substr(idx);
  if (SUFFIX_TYPE.count(suffix) == 1) {
    return SUFFIX_TYPE[suffix];
  }
  return plainStr;
}

} // namespace
std::string HttpResponse::srcDir_ = "resources/";

void HttpResponse::fillBuf() {
  LOG_DEBUG("source dir : %s, file path : %s", srcDir_.c_str(), path_.c_str());
  if (statusCode_ != k400BadRequest) {
    if (stat((srcDir_ + path_).data(), &mmFileStat_) < 0 
      || S_ISDIR(mmFileStat_.st_mode)) {
      setStatusCode(HttpResponse::k404NotFound);
    } else if (!(mmFileStat_.st_mode & S_IROTH)) {
      setStatusCode(HttpResponse::k403Forbidden);
    } else {
      setStatusCode(HttpResponse::k200Ok);
    }   
  }
  setErrorHtml();
  addStateLineAppendBuf();
  addHeaderAppendBuf();
  addContentAppendBuf();
}

void HttpResponse::setErrorHtml() {
  auto it = CODE_PATH.find(statusCode_);
  if (it != CODE_PATH.end()) {
    path_ = it->second;
    stat((srcDir_ + path_).data(), &mmFileStat_);
  }
}
void HttpResponse::addStateLineAppendBuf() {
  buf.append("HTTP/1.1 " + to_string(statusCode_) + " " +  CODE_STATUS[statusCode_]+ "\r\n");
}

void HttpResponse::addHeaderAppendBuf() {
  buf.append("Connection: ");
  if (closeConnection_) {
    buf.append("close\r\n");
  } else {
    buf.append("keep-alive\r\n");
    buf.append("keep-alive: max=6, timeout=120\r\n");
  }
  buf.append("Content-type: " + getFileType(path_) + "\r\n");
}

void HttpResponse::addContentAppendBuf() {
  int fd = open((srcDir_ + path_).data(), O_RDONLY);
  if (fd < 0) {
    errorContent("File NotFound!");
    return;
  }
  buf.append("Content-length: " + to_string(mmFileStat_.st_size) + "\r\n\r\n");
  int saveError;
  buf.readFd(fd, &saveError);
}

void HttpResponse::errorContent(std::string&& message) {
  string body;
  string status;
  body += "<html><title>Error</title>";
  body += "<body bgcolor=\"ffffff\">";
  if(CODE_STATUS.count(statusCode_) == 1) {
      status = CODE_STATUS.find(statusCode_)->second;
  } else {
      status = "Bad Request";
  }
  body += to_string(statusCode_) + " : " + status  + "\n";
  body += "<p>" + message + "</p>";
  body += "<hr><em>TinyWebServer</em></body></html>";

  buf.append("Content-length: " + to_string(body.size()) + "\r\n\r\n");
  buf.append(body); 
}
