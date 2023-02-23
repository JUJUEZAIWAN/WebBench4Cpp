
## WebBench4Cpp
[简体中文](README_CN.md) | [English](./README.md)

WebBench4Cpp 是一个用于测试web服务器性能的工具。它是用C++编写的，基于C语言的[WebBench](http://home.tiscali.cz/~cz210552/webbench.html)工具改进而来。



不同于WebBench，WebBench4Cpp是一个多线程的工具。它可以用于测试web服务器在高并发下的性能。此外，WebBench4Cpp支持HTTPS协议和epoll IO多路复用。

### Usage
```
Usage: webbench [option]... URL
  -t|--time <sec>           运行测试的时间，单位秒，默认30秒。
  
  -v|--http version <1.0|1.1|2.0>
                            使用的HTTP协议版本，1.0或1.1或2.0。
  -c|--clients <n>          同时运行的客户端数量，默认256。
  -r|--recv data <true|false>        
                            是否接收服务器返回的数据，默认false。
  -m|--method <method>     HTTP方法，默认GET。
  -u|--uri <uri>           HTTP uri。url的路径不能为空。
```

### Example
```
./WebBench4Cpp -c 4096 -t 60 -u http://www.baidu.com/  -r true -v 1.1 -m GET 
# 4096 clients, 60 seconds, receive data, http version 1.1, GET method
```

```
./WebBench4Cpp -c 1024 -t 120 -u https://www.baidu.com/  -r true -v 1.1 -m GET 
# 1024 clients, 120 seconds, receive data, http version 1.1, GET method, 
```

```
./WebBench4Cpp -c 1024 -t 120 -u https://www.baidu.com/  -r false -v 1.1 -m GET 
# 1024 clients, 120 seconds, not receive data, http version 1.1, GET method,
```

### Build
```
cd src
sh build.sh
# 或者使用xmake工具构建 (https://xmake.io/#/zh-cn/)
xmake 
```



### TODO
- [ ] 添加更多的HTTP方法
- [ ] 格式化输出
- [ ] 添加更多的选项，比如从文件中加载http头，等等。
- [ ] 添加更多的输出信息，比如平均响应时间，等等。