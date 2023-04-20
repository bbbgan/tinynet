## *Introduce*

​	**tinynet**是一个基于多**Reactor**模式的多线程C++网络库，仿照[Muduo](https://github.com/chenshuo/muduo)网络库进行编写：

+ 使用std::chrono代替封装的底层时间类，可以更方便的使用各种时间单位， e.g.

  ```
  #include <tinynet/net/EventLoop.h>
  using namespace tinynet;
  using namespace tinynet::net;
  
  loop.runAfter(1h, [](){LOG_INFO("run after one hour");} );
  ```

+ 使用C++11标准线程库，而不是封装的POSIX thread API。

+ 使用C print风格的异步以及同步日志

+ 内置简单的HTTP、EchoServer、ChatServer等示例程序

## *Install*

```
$ git clone git@github.com:bbbgan/tinynet.git
$ cd tinynet && ./build.sh
```

## *More*

关于更多项目细节移步[bblog](https://bbbgan.github.io/)

## *TODO*

+ 异步日志性能优化
+ HTTP服务器测试
+ RPC框架



