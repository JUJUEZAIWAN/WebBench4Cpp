

#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cstring>
#include <memory>

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

#include "noCopy.h"
#include <iostream>
#include "Socket.h"

namespace WebBench
{

    using std::string;


    class Client
    {
    public:
        Client(int socknum, bool ssl = false) : socknum_(socknum), is_ssl_(ssl),socket_(new EpollSocket(socknum, ssl))
        {
            
        }
        ~Client()
        {

        }

        DISABLE_COPY(Client)

    public:
        void close();

        bool connect(const string &host, int port);

        void send(const string &requestdata, bool &flag);

        void recv(bool &flag);
        size_t getRecvLen();
        size_t getSendLen();

        size_t getSocketNum() const;
        int getSocketSendNumError();
        int getSocketRecvNumError();

    private:
        int socknum_;
        bool is_ssl_;
        std::unique_ptr<EpollSocket> socket_;
    };

} // namespace WebBench
#endif
