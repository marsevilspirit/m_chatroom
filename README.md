# m_chatroom

#### 简介

这是一个聊天室服务器和客户端，用于练习`linux`网络编程，数据库基本操作，`C++`，`Cmake`...

服务器和客户端均基于我自己学习`muduo`网络库所写的自己的`m_netlib`网络库构建。

聊天室的服务器基于TCP协议，多线程多`epoll`，线程池管理多线程处理请求的主从多`reactor`模型。

客户端，两个线程，`EventLoop`独占一个线程处理网络IO，另一个线程负责读键盘，采用不阻塞的方式与服务器连接。

数据通过`mysql`进行管理和操作, `redis`作为缓存, 使用`json`序列化和反序列化。

C++书写，通过`Cmake`管理多文件编译。

代码量8000行左右。



#### 开发环境

- 操作系统：archlinux 6.10.4-arch2-1
- 编译器：clang++ version 18.1.8
- 文本编辑器：neovim v0.10.1
- 构建工具：CMake



#### 依赖

`archlinux`:

```
sudo pacman -S openssl hiredis mysql-clients fmt jsoncpp
```

使用了`nlohmann`的`json`库



#### 目录结构

```
                                                                       .
├── CMakeLists.txt
├── project
│   ├── chatclient.cc
│   ├── chatroom.cc
│   ├── client
│   │   ├── client.cc
│   │   ├── client.h
│   │   ├── clientService.cc
│   │   ├── clientService.h
│   │   ├── CMakeLists.txt
│   │   ├── util.cc
│   │   └── util.h
│   ├── ClientJsonCodec.h
│   ├── database
│   │   ├── CMakeLists.txt
│   │   ├── database.cc
│   │   ├── database.h
│   │   ├── mysqlPool.cc
│   │   ├── mysqlPool.h
│   │   ├── redis.cc
│   │   ├── redis.h
│   │   ├── redisPool.cc
│   │   └── redisPool.h
│   ├── json.hpp
│   ├── m_netlib
│   │   ├── Base
│   │   │   ├── copyable.h
│   │   │   ├── noncopyable.h
│   │   │   ├── Timestamp.cc
│   │   │   └── Timestamp.h
│   │   ├── CMakeLists.txt
│   │   ├── Log
│   │   │   ├── logconf.json
│   │   │   ├── mars_logger.cc
│   │   │   └── mars_logger.h
│   │   ├── Net
│   │   │   ├── Acceptor.cc
│   │   │   ├── Acceptor.h
│   │   │   ├── Buffer.cc
│   │   │   ├── Buffer.h
│   │   │   ├── Callbacks.h
│   │   │   ├── Channel.cc
│   │   │   ├── Channel.h
│   │   │   ├── Connector.cc
│   │   │   ├── Connector.h
│   │   │   ├── EPoller.cc
│   │   │   ├── EPoller.h
│   │   │   ├── EventLoop.cc
│   │   │   ├── EventLoop.h
│   │   │   ├── EventLoopThread.cc
│   │   │   ├── EventLoopThread.h
│   │   │   ├── EventLoopThreadPool.cc
│   │   │   ├── EventLoopThreadPool.h
│   │   │   ├── Poller.cc
│   │   │   ├── Poller.h
│   │   │   ├── TcpClient.cc
│   │   │   ├── TcpClient.h
│   │   │   ├── TcpConnection.cc
│   │   │   ├── TcpConnection.h
│   │   │   ├── TcpServer.cc
│   │   │   ├── TcpServer.h
│   │   │   ├── Timer.cc
│   │   │   ├── Timer.h
│   │   │   ├── TimerId.h
│   │   │   ├── TimerQueue.cc
│   │   │   └── TimerQueue.h
│   │   └── Socket
│   │       ├── InetAddress.cc
│   │       ├── InetAddress.h
│   │       ├── Socket.cc
│   │       ├── Socket.h
│   │       ├── SocketOps.cc
│   │       └── SocketOps.h
│   ├── model
│   │   ├── CMakeLists.txt
│   │   ├── file.h
│   │   ├── filemodel.cc
│   │   ├── filemodel.h
│   │   ├── friendmodel.cc
│   │   ├── friendmodel.h
│   │   ├── groupchathistory.h
│   │   ├── group.h
│   │   ├── groupmodel.cc
│   │   ├── groupmodel.h
│   │   ├── groupuser.h
│   │   ├── historyCasheManager.h
│   │   ├── historymodel.cc
│   │   ├── historymodel.h
│   │   ├── privatechathistory.h
│   │   ├── user.h
│   │   ├── usermodel.cc
│   │   └── usermodel.h
│   ├── public.h
│   ├── server
│   │   ├── CMakeLists.txt
│   │   ├── server.cc
│   │   ├── server.h
│   │   ├── service.cc
│   │   └── service.h
│   ├── ServerJsonCodec.h
│   └── threadpool
│       ├── CMakeLists.txt
│       ├── threadpool.cc
│       └── threadpool.h
├── README.md
├── setup_debug.sh
└── setup.sh

12 directories, 96 files

```

#### 代码量

```
-------------------------------------------------------------------------------
Language                     files          blank        comment           code
-------------------------------------------------------------------------------
C++                             36            973            256           4592
C/C++ Header                    48            701            100           1928
CMake                            7             26              8            148
Markdown                         1             21              0            114
Bourne Shell                     2              7              0             20
JSON                             1              0              0             11
-------------------------------------------------------------------------------
SUM:                            95           1728            364           6813
-------------------------------------------------------------------------------
```

