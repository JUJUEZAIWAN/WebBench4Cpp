
#include "client.h"

using namespace WebBench;

bool Client::connect(const string &host, int port)
{
    if (socket_ == nullptr)
    {
        return false;
    }
    return socket_->connect(host, port);
}

void Client::send(const string &data, bool &is_stop)
{
    if (socket_ == nullptr)
    {
        return;
    }
    socket_->send(data, is_stop);
}

void Client::recv(bool &is_stop)
{
    if (socket_ == nullptr)
    {
        return;
    }
    socket_->recv(is_stop);
}

size_t Client::getRecvLen()
{
    if (socket_ == nullptr)
    {
        return 0;
    }
    return socket_->getRecvLen();
}

size_t Client::getSendLen()
{
    if (socket_ == nullptr)
    {
        return 0;
    }
    return socket_->getSendLen();
}

size_t Client::getSocketNum() const
{
    return socknum_;
}

int Client::getSocketSendNumError()
{
    if(socket_ == nullptr)
        return 0;
    return socket_->getSocketSendNumError();
}
int Client::getSocketRecvNumError()
{
    if(socket_ == nullptr)
        return 0;
    return socket_->getSocketRecvNumError();
}

