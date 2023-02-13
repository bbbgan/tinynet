## *Introduce*

​	**tinynet**是一个基于**Reactor**模型的多线程C++网络库，仿照[Muduo](https://github.com/chenshuo/muduo)[网络库进行编写：

+ 使用std::chrono代替封装的底层时间类，可以更方便的使用各种时间单位， e.g.

  ```
  #include <tinynet/net/EventLoop.h>
  using namespace tinynet;
  using namespace tinynet::net;
  
  loop.runAfter(1h, [](){LOG_INFO("run after one hour");} );
  ```

+ 使用C++11标准线程库，而不是封装的POSIX thread API。

+ 使用C print风格的日志

+ 内置简陋的HTTP服务器例程

## *Install*

```
$ git clone git@github.com:bbbgan/tinynet.git
$ cd tinynet && ./build.sh
```

## *More*



## *TODO*

+ 高性能异步日志
+ HTTP服务器测试
+ RPC框架



