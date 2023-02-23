
#include <fstream>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "request.h"

using namespace WebBench;

std::unordered_map<HTTP_VERSION, std::string> version_map = {
    {HTTP_1_0, "HTTP/1.0"},
    {HTTP_1_1, "HTTP/1.1"},
    {HTTP_2_0, "HTTP/2.0"}};

std::unordered_map<HTTP_METHOD, std::string> method_map = {
    {GET, "GET"},
    {POST, "POST"},
    {HEAD, "HEAD"},
    {PUT, "PUT"},
    {DELETE, "DELETE"},
    {TRACE, "TRACE"},
    {OPTIONS, "OPTIONS"},
    {CONNECT, "CONNECT"},
    {PATCH, "PATCH"}};

Request::Request(const string &host, const string &path, HTTP_METHOD method, HTTP_VERSION version)
    : host_(host), path_(path), method_(method), version_(version), header_(""), body_("")
{
}

void Request::defaultHeader()
{

    header_ = "";
    header_ += method_map[method_] + " " + path_ + " " + version_map[version_] + "\r\n";
    header_ += "Host: " + host_ + "\r\n";
    if (version_ >= HTTP_1_1)
    {
        header_ += "Connection: keep-alive\r\n";
    }

    header_ += "Accept: */*\r\n";
    header_ += "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/83.0.4103.116 Safari/537.36\r\n";
    header_ += "Accept-Encoding: gzip, deflate, br\r\n";
    header_ += "Accept-Language: zh-CN,zh;q=0.9\r\n";
    header_ += "\r\n";
}

void Request::addHeader(const string &key, const string &value)
{
    header_ += key;
    header_ += ":";
    header_ += value;
    header_ += "\r\n";
}

void Request::addBody(const string &body)
{
    body_ = body;
}
string Request::getRequest() const
{
    return header_ + body_;
}
void Request::clear()
{
    header_.clear();
    body_.clear();
    host_.clear();
    path_.clear();
}
void Request::addHeader(const string &header)
{
    header_ += header;
}

void Request::addHeaderFromFile(const string &file_path)
{
    std::fstream file(file_path, std::ios::in);
    if (!file.is_open())
    {
        std::cout << "file open failed" << std::endl;
        return;
    }
    std::string line;
    while (std::getline(file, line))
    {
        header_ += line;
        if (line[line.size() - 1] != '\r' && line[line.size() - 1] != '\n')
        {
            header_ += "\r\n";
        }
    }
    std::cout << header_.size() << std::endl;
}

bool Request::parse_url(const string &url, string &host, string &path, int &port, bool &is_https)
{
    // parse url

    if (url.find("http://") == 0)
    {
        is_https = false;
    }
    else if (url.find("https://") == 0)
    {
        is_https = true;
    }
    else
    {
        std::cout << "url is not http or https" << std::endl;
        return false;
    }

    string::size_type pos = is_https ? 8 : 7;
    string::size_type pos2 = url.find('/', pos);
    if (pos2 == string::npos)
    {
        std::cout << "url is not complete " << std::endl;
        return false;
    }
    path = url.substr(pos2);
    host = url.substr(pos, pos2 - pos);
    string::size_type pos3 = host.find(':');
    if (pos3 == string::npos)
    {
        port = is_https ? 443 : 80;
    }
    else
    {
        port = std::stoi(host.substr(pos3 + 1));
        host = host.substr(0, pos3);
    }

    return true;
}

string Request::host2ip(const string &host)
{
    if (host.empty())
    {
        std::cout << "host is empty" << std::endl;
        exit(1);
    }
    // check host is ip or domain
    struct in_addr addr;
    if (inet_aton(host.c_str(), &addr))
    {
        return host;
    }

    struct hostent *hptr = ::gethostbyname(host.c_str());
    if (hptr == nullptr)
    {
        std::cout << "gethostbyname error for host:" << host << std::endl;
        return "";
    }
    char str[32];
    inet_ntop(hptr->h_addrtype, hptr->h_addr, str, sizeof(str));
    return str;
}