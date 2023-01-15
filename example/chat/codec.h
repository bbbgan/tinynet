#ifndef TINYNET_EXAMPLE_CHAT_CODEC_H_
#define TINYNET_EXAMPLE_CHAT_CODEC_H_

#include <tinynet/net/EventLoop.h>
#include <tinynet/net/TcpConnection.h>

class LengthHeaderCodec : tinynet::noncopyable
{
 public:
  typedef std::function<void (const tinynet::net::TcpConnectionPtr&,
                              const std::string& msg,
                              tinynet::Timestamp)> MessageCompleteCallback; 
  explicit LengthHeaderCodec(const MessageCompleteCallback& cb)
    : messageCompleteCallback_(cb) {}
  void decodeMessage(const tinynet::net::TcpConnectionPtr& conn,
                 tinynet::net::Buffer* buf,
                 tinynet::Timestamp receiveTIme) 
  {
    while (buf->readableBytes() >=kHeaderLen) {
      const void* data = buf->peek();
      int32_t bigEnd32 = *static_cast<const int32_t*>(data);
      const int32_t len = be32toh(bigEnd32);
      if (len > 65536 || len < 0) {
        assert(0);  // FIXME:
        conn->shutdown();  // FIXME: disable reading
        break;
      } else if (buf->readableBytes() >= len + kHeaderLen) {
        buf->retrieve(kHeaderLen);
        std::string msg(buf->peek(), len);
        messageCompleteCallback_(conn, msg, receiveTIme);
        buf->retrieve(len);
      } else {
        break;
      }
    }
  }
  void encodeMessage(tinynet::net::TcpConnection* conn,
            const std::string& msg) 
{
  tinynet::net::Buffer buf;
  buf.append(msg.data(), msg.size());
  int32_t len = static_cast<int32_t>(msg.size());
  int32_t be32 = htobe32(len);
  buf.prepend(&be32, sizeof be32);
  conn->send(&buf);
}
 private:
  MessageCompleteCallback messageCompleteCallback_;
  const static size_t kHeaderLen = sizeof(int32_t);

};


#endif // TINYNET_EXAMPLE_CHAT_CODEC_H_