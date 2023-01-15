set(net_SourceFile
  Acceptor.cc
  Buffer.cc
  Poller.cc
  Channel.cc
  TimerQueue.cc
  EventLoop.cc
  EventLoopThread.cc
  EventLoopThreadPool.cc
  Channel.cc
  InetAddress.cc
  Socket.cc
  TcpConnection.cc
  TcpServer.cc
  Connector.cc
  TcpClient.cc
  )

add_library(tinynet_net ${net_SourceFile})
target_link_libraries(tinynet_net tinynet_base pthread)

# #add_library(muduo_net_cpp11 ${net_SRCS})
# #target_link_libraries(muduo_net_cpp11 muduo_base_cpp11)
# #set_target_properties(muduo_net_cpp11 PROPERTIES COMPILE_FLAGS "-std=c++0x")

# install(TARGETS muduo_net DESTINATION lib)
# #install(TARGETS muduo_net_cpp11 DESTINATION lib)

# set(HEADERS
#   Buffer.h
#   Callbacks.h
#   Channel.h
#   Endian.h
#   EventLoop.h
#   EventLoopThread.h
#   EventLoopThreadPool.h
#   InetAddress.h
#   TcpClient.h
#   TcpConnection.h
#   TcpServer.h
#   TimerId.h
#   )
# install(FILES ${HEADERS} DESTINATION include/muduo/net)

