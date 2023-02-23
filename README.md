# WebBench4Cpp
[简体中文](README_CN.md) | [English](./README.md)

WebBench4Cpp is a tool for benchmarking web servers. It is written in C++ and is based on the [WebBench](http://home.tiscali.cz/~cz210552/webbench.html) tool written in C.

Different from WebBench, WebBench4Cpp is a multi-threaded tool . It can be used to test the performance of web servers under high concurrency.Besides, WebBench4Cpp supports HTTPS protocol and epoll IO multiplexing.

### Usage
```
Usage: webbench [option]... URL
  -t|--time <sec>          Run benchmark for <sec> seconds. Default 30.
  -v|--http version <1.0|1.1|2.0>
                           Use HTTP protocol version 1.0 or 1.1 or 2.0.
  -c|--clients <n>         Run <n> HTTP clients at once. Default 256.
  -r|--recv data <true|false>        
                           Receive data from server. Default false.
  -m|--method <method>     HTTP method. Default GET.
  -u|--uri <uri>           HTTP uri. the path of the url must not be empty.
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
#or use xmake build tool (https://xmake.io/#/zh-cn/)
xmake 
```



### TODO
- [ ] Add more HTTP methods
- [ ] Format the output
- [ ] Add more options,such as load http header from file,etc.
- [ ] Add more information in the output,such as the average response time,etc.


