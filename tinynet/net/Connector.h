// 
// Author       : gan
// Date         : 2022-09-10
//
#ifndef TINYNET_NET_CONNECTOR_H_
#define TINYNET_NET_CONNECTOR_H_
#include <memory>
#include <functional>

#include <tinynet/base/noncopyable.h>
#include <tinynet/net/Callbacks.h>
#include <tinynet/net/InetAddress.h>

namespace tinynet
{
namespace net
{
class Channel;
class EventLoop;
class Connector : noncopyable,
                  public std::enable_shared_from_this<Connector>
{
public: 
  Connector(EventLoop* loop, const InetAddress& serverAddr);
  ~Connector();
  void start();  // can be called in any thread
  void setNewConnectionCallback(const NewConnectionCallback& cb)
  { newConnectionCallback_ = cb; }

  const InetAddress& peerAddress() const { return peerAddr_; }
  int removeAndResetChannel();

 private:
  enum ConnectionState { kConnecting, kConnected, kDisconnecting, kDisconnected };
  int stateAtomicGetAndSet(int newState){
    return __atomic_exchange_n(&state_, newState, __ATOMIC_SEQ_CST);
  }
  void startInLoop();
  void connect();
  void connecting(int sockfd);
  void handleWrite();
  void handleError();
  void retry(int sockfd);

  EventLoop* loop_;
  InetAddress peerAddr_;
  bool startconnect_; 
  std::unique_ptr<Channel> channel_;
  NewConnectionCallback newConnectionCallback_;
  int retryDelayMs_;
  int state_;
};
} // namespace net

} // namespace tinynet


#endif // TINYNET_NET_CONNECTOR_H_