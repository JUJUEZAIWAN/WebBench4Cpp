
#ifndef REQUEST_H
#define REQUEST_H

#include <string>
#include <unordered_map>
#include <iostream>
#include "noCopy.h"

namespace WebBench
{

    using std::string;

    enum HTTP_METHOD
    {
        GET,
        POST,
        HEAD,
        PUT,
        DELETE,
        TRACE,
        OPTIONS,
        CONNECT,
        PATCH
    };

    

    enum HTTP_VERSION
    {
        HTTP_1_0,
        HTTP_1_1,
        HTTP_2_0
    };

    

    class Request
    {
    public:
        Request(const string &host, const string &path, HTTP_METHOD method, HTTP_VERSION version);
        ~Request() = default;
        DISABLE_COPY(Request)

    public:
        void defaultHeader();

        void addHeader(const string &key, const string &value);
        void addBody(const string &body);
        string getRequest() const;
        void clear();
        void addHeader(const string &header);
        void addHeaderFromFile(const string &file_path);

        static bool parse_url(const string &url, string &host, string &path, int &port, bool &is_https);
        static string host2ip(const string &host);


    private:
        string host_;
        string path_;
        HTTP_METHOD method_;
        HTTP_VERSION version_;
        string header_;
        string body_;
    };

    

} // namespace WebBench
#endif