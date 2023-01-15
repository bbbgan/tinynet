#ifndef TINYNET_CALLLBACKS_H_
#define TINYNET_CALLLBACKS_H_

#include <memory>
#include <functional>
#include <tinynet/base/Timestamp.h>

namespace tinynet
{
namespace net
{
  
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;
using std::placeholders::_5;

class Buffer;
class TcpConnection;
class InetAddress;
class EventLoop;

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
typedef std::function<void(const TcpConnectionPtr&)> CloseCallback;
typedef std::function<void(const TcpConnectionPtr&)> ConnectionCallback;
typedef std::function<void(const TcpConnectionPtr&)> WriteCompleteCallback;
typedef std::function<void(const TcpConnectionPtr&, size_t)> HighWaterMarkCallback;
typedef std::function<void(const TcpConnectionPtr&, Buffer*, Timestamp)> MessageCallback;

typedef std::function<void()> ErrorCallback;
typedef std::function<void(int sockfd,
                           const InetAddress& peer)> NewConnectionCallback;

typedef std::function<void()> Task;
typedef std::function<void(EventLoop* loop)> ThreadInitCallback;
typedef std::function<void()> TimerCallback;

void defaultThreadInitCallback(EventLoop* loop);
void defaultConnectionCallback(const TcpConnectionPtr& conn);
void defaultMessageCallback(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp);
}
} // namesapce tinynet

#endif // TINYNET_CALLLBACKS_H_
