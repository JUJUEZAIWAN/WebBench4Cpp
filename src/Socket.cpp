

#include "Socket.h"
#include <iostream>
using namespace WebBench;

using std::string;
using std::vector;

EpollSocket::EpollSocket(int socknum, bool ssl) : socknum_(socknum), is_ssl_(ssl), epollfd_(-1), recv_epollfd_(-1), send_epollfd_(-1),
                                                  conn_event_(nullptr), recv_event_(nullptr), send_event_(nullptr),
                                                  recv_len_(0), send_len_(0), recv_error_(0), send_error_(0),
                                                  socketfds_(), socketfd_ssl_(), ctx_(nullptr)
{
}

EpollSocket::~EpollSocket()
{
    close();
}

void EpollSocket::send(const string &data, bool &is_stop)
{
    if (socketfds_.size() == 0 || is_stop)
        return;
    if (!register_event(send_epollfd_, EPOLLOUT, &send_event_))
    {
        return;
    }

    int nfds = epoll_wait(send_epollfd_, send_event_, socketfds_.size(), 100);
    if (nfds == -1)
    {
        return;
    }

    size_t len = data.size();
    for (int i = 0; i < nfds; ++i)
    {
        int socketfd = send_event_[i].data.fd;
        if (send_event_[i].events & EPOLLOUT)
        {
            SSL *ssl_ = socketfd_ssl_[socketfd];
            int send_ret = 0;
            long send_len = 0;
            while (1)
            {
                if (is_ssl_)
                {
                    send_ret = SSL_write(ssl_, data.c_str() + send_len, data.size() - send_len);
                }
                else
                {
                    send_ret = ::send(socketfd, data.c_str() + send_len, data.size() - send_len, 0);
                }
                if (send_ret == -1)
                {
                    if (errno == EINTR || errno == EWOULDBLOCK)
                    {
                        break;
                    }
                    else if (errno == EAGAIN)
                    {
                        continue;
                    }
                    else
                    {
                        send_error_++;
                        delete_error_sock(socketfd, false);
                        socketfd_ssl_.erase(socketfd);
                        socketfds_.erase(remove(socketfds_.begin(), socketfds_.end(), socketfd), socketfds_.end());
                        break;
                    }
                }
                else if (send_ret == static_cast<ssize_t>(len - send_len))
                {
                    send_len += send_ret;
                    break;
                }
                else
                {
                    send_len += send_ret;
                }
            }
            if (send_len > 0)
            {
                send_len_ += send_len;
            }
        }
        if (is_stop)
            return;
    }
}
void EpollSocket::recv(bool &is_stop)
{
    if (socketfds_.size() == 0 || is_stop)
        return;

    if (!register_event(recv_epollfd_, EPOLLIN, &recv_event_))
    {
        return;
    }

    char buf[8192];

    int nfds = epoll_wait(recv_epollfd_, recv_event_, socketfds_.size(), 100);
    if (nfds == -1)
    {
        free_event(&recv_event_);
        close_epoll(recv_epollfd_);
        return;
    }

    for (int i = 0; i < nfds; i++)
    {
        int socketfd = recv_event_[i].data.fd;
        if (recv_event_[i].events & EPOLLIN)
        {
            SSL *ssl_ = socketfd_ssl_[socketfd];
            int recv_ret = 0;
            long recv_len = 0;
            while (1)
            {
                if (is_ssl_)
                {
                    recv_ret = SSL_read(ssl_, buf, sizeof(buf));
                }
                else
                {
                    recv_ret = ::recv(socketfd, buf, sizeof(buf), 0);
                }
                if (recv_ret == -1)
                {
                    if (errno == EWOULDBLOCK || errno == EINTR)
                    {
                        break;
                    }
                    else if (errno == EAGAIN)
                    {
                        continue;
                    }
                    else
                    {
                        recv_error_++;
                        delete_error_sock(socketfd, true);
                        socketfd_ssl_.erase(socketfd);
                        socketfds_.erase(remove(socketfds_.begin(), socketfds_.end(), socketfd), socketfds_.end());
                        break;
                    }
                }
                else if (recv_ret == 0)
                {
                    recv_error_++;
                    delete_error_sock(socketfd, true);
                    socketfd_ssl_.erase(socketfd);
                    socketfds_.erase(remove(socketfds_.begin(), socketfds_.end(), socketfd), socketfds_.end());
                    break;
                }
                else if (static_cast<size_t>(recv_ret) < sizeof(buf))
                {
                    recv_len += recv_ret;
                    break;
                }
                else
                {
                    recv_len += recv_ret;
                }
            }
            if (recv_len > 0)
            {
                recv_len_ += recv_len;
            }
        }
        if (is_stop)
            return;
    }
}
bool EpollSocket::connect(const string &host, int port)
{
    if (is_ssl_)
        return ssl_connect(host, port);
    else
        return tcp_connect(host, port);
}

size_t EpollSocket::getRecvLen()
{
    auto tmp = recv_len_;
    recv_len_ = 0;
    return tmp;
}
size_t EpollSocket::getSendLen()
{
    auto tmp = send_len_;
    send_len_ = 0;
    return tmp;
}

int EpollSocket::getSocketNum() const
{
    return socketfds_.size();
}
int EpollSocket::getSocketRecvNumError()
{
    auto tmp = recv_error_;
    recv_error_ = 0;
    return tmp;
}
int EpollSocket::getSocketSendNumError()
{
    auto tmp = send_error_;
    send_error_ = 0;
    return tmp;
}

bool EpollSocket::tcp_connect(const string &host, int port)
{

    if (!noBlockConnect(host, port))
    {
        
        return false;
    }
    
    register_event(epollfd_, EPOLLOUT, &conn_event_);

    int nfds = epoll_wait(epollfd_, conn_event_, socketfds_.size(), 3000);
    if (nfds <=0)
    {
        free_event(&conn_event_);
        close_epoll(epollfd_);
        return false;
    }

    vector<int> socketfds;
    for (int i = 0; i < nfds; i++)
    {
        int socketfd = conn_event_[i].data.fd;
        if (conn_event_[i].events & EPOLLOUT)
        {        
            socketfds.push_back(socketfd);
        }
    }
    free_event(&conn_event_);
    close_epoll(epollfd_);
    if (socketfds.size() == 0)
    {
        return false;
    }
    else
    {
        socketfds_.swap(socketfds);
        return true;
    }
}

bool EpollSocket::ssl_connect(const string &host, int port)
{
    SSL_library_init();
    SSL_load_error_strings();
    ctx_ = SSL_CTX_new(SSLv23_client_method());
    if (!tcp_connect(host, port))
    {
        return false;
    }
    std::vector<int> socketfds;

    SSL *ssl_;
    unsigned long ul = 0;
    for (auto &socketfd : socketfds_)
    {
        ioctl(socketfd, FIONBIO, &ul);
        ssl_ = SSL_new(ctx_);
        if (ssl_ == nullptr)
        {
            // std::cout << "SSL_new failed" << std::endl;
            continue;
        }
        SSL_set_fd(ssl_, socketfd); // FIXME not check return value ,some error happend

        int ret = SSL_connect(ssl_);
        if (ret == -1)
        {
            // std::cout << "SSL_connect failed" <<SSL_get_error(ssl_,ret) <<std::endl;
            continue;
        }
        socketfds.push_back(socketfd);

        socketfd_ssl_[socketfd] = ssl_;
    }

    socketfds_.swap(socketfds);
    for (auto &sock : socketfds_)
    {
        ul = 1;
        ioctl(sock, FIONBIO, &ul);
    }

    return true;
}
bool EpollSocket::noBlockConnect(const string &host, int port)
{
    for (int i = 0; i < socknum_; i++)
    {
        int socketfd = socket(AF_INET, SOCK_STREAM, 0);
        if (socketfd < 0)
        {
            continue;
        }
        else
        {

            struct sockaddr_in addr;
            memset(&addr, 0, sizeof(addr));
            addr.sin_family = AF_INET;

            addr.sin_port = htons(port);
            addr.sin_addr.s_addr = inet_addr(host.c_str());

            unsigned long ul = 1;
            if (ioctl(socketfd, FIONBIO, &ul) < 0)
            {
                ::close(socketfd);
                continue;
            }

            auto connect_ret = ::connect(socketfd, (struct sockaddr *)&addr, sizeof(addr));
            if (connect_ret == -1)
            {
                if (errno == EINPROGRESS)
                {
                    socketfds_.push_back(socketfd);
                }
                else
                {
                    ::close(socketfd);
                }
                continue;
            }
            else if (connect_ret == 0)
            {
                socketfds_.push_back(socketfd);
            }
        }
    }
    if (socketfds_.empty())
    {
        return false;
    }
    else
    {
        return true;
    }
}

void EpollSocket::close_epoll(int &socket)
{
    if (socket != -1)
    {
        ::close(socket);
        socket = -1;
    }
}
void EpollSocket::delete_error_sock(int socket, bool is_read)
{
    epoll_event ev;
    ev.events = is_read ? EPOLLIN : EPOLLOUT;
    ev.data.fd = socket;
    epoll_ctl(is_read ? recv_epollfd_ : send_epollfd_, EPOLL_CTL_DEL, socket, &ev);
}

void EpollSocket::close_single(int socketfd)
{
    if (is_ssl_)
    {
        SSL *ssl = socketfd_ssl_[socketfd];
        SSL_shutdown(ssl);
        SSL_free(ssl);
    }
    ::close(socketfd);
}
void EpollSocket::free_event(epoll_event **event)
{
    if (*event != nullptr)
    {
        delete[] * event;
        *event = nullptr;
    }
}


bool EpollSocket::register_event(int &epollfd, uint32_t events_type, epoll_event **events)
{
    if (epollfd == -1)
    {
        epollfd = epoll_create(socketfds_.size());
        if (epollfd == -1)
        {
            return false;
        }
        *events = new epoll_event[socketfds_.size()];
        for (auto &socketfd : socketfds_)
        {
            struct epoll_event ev;
            ev.events = events_type;
            ev.data.fd = socketfd;
            epoll_ctl(epollfd, EPOLL_CTL_ADD, socketfd, &ev);
        }
    }
    return true;
}


void EpollSocket::close()
{
    for (auto &socketfd : socketfds_)
    {
        close_single(socketfd);
    }
    socketfds_.clear();
    socketfd_ssl_.clear();
    close_epoll(epollfd_);
    close_epoll(recv_epollfd_);
    close_epoll(send_epollfd_);
    free_event(&conn_event_);
    free_event(&recv_event_);
    free_event(&send_event_);
    if (is_ssl_)
    {
        SSL_CTX_free(ctx_);
    }
}
