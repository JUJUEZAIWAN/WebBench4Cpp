

#ifndef SOCKET_H
#define SOCKET_H

#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cstring>

#include <sys/types.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/epoll.h>

#include "noCopy.h"
#include <iostream>

namespace WebBench
{
    using std::string;
    using std::vector;
    class EpollSocket
    {
    public:
        EpollSocket(int socknum, bool ssl);

        ~EpollSocket();
        DISABLE_COPY(EpollSocket)

    public:
        void send(const string &data, bool &is_stop);
        void recv(bool &is_stop);
        bool connect(const string &host, int port);

        size_t getRecvLen();
        size_t getSendLen();

        int getSocketNum() const;
        int getSocketRecvNumError();
        int getSocketSendNumError();

    private:
        bool tcp_connect(const string &host, int port);

        bool ssl_connect(const string &host, int port);
        
        bool noBlockConnect(const string &host, int port);
       

        void close_epoll(int &socket);
        
        void delete_error_sock(int socket, bool is_read);
        

        void close_single(int socketfd);
      
        void free_event(epoll_event **event);
       
        bool register_event(int &epollfd, uint32_t events_type, epoll_event ** events);
        void close();
        

    private:
        int socknum_;
        bool is_ssl_;
        int epollfd_;
        int recv_epollfd_;
        int send_epollfd_;
        epoll_event *conn_event_;
        epoll_event *recv_event_;
        epoll_event *send_event_;

        int send_len_;
        int recv_len_;
        int send_error_;
        int recv_error_;
        std::vector<int> socketfds_;
        std::unordered_map<int, SSL *> socketfd_ssl_;
        SSL_CTX *ctx_;
    };

} // namespace Webbench
#endif
