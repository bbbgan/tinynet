TcpServer
  acceptor 
    InetAddress  
    Socket  
  EventLoopThread 
    EventLoop   
      Channel  
      Poller  
      TimerQueue 
        Timer  自删除
  TcpConnection   stateAtomicGetAndSet 
  callback 
TcpClient  thread safe 
  Connector connecting的FIXME
Log  ok 后面做异步日志 太长放不下

errno 是线程安全的
string_view
生命期以及线程安全性
shared_from_this
nc -nv  127.0.0.1 9999
https://zhuanlan.zhihu.com/p/159130182
strace命令：https://www.cnblogs.com/machangwei-8/p/10388883.html