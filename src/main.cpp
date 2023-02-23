
#include <iostream>
#include <cassert>
#include "cmdline.h"
#include "request.h"
#include "bench.h"

using namespace std;
using namespace WebBench;
using namespace cmdpaser;

std::unordered_map<string, HTTP_VERSION> version_rmap = {
    {"1.0", HTTP_1_0},
    {"1.1", HTTP_1_1},
    {"2.0", HTTP_2_0}};
std::unordered_map<string, HTTP_METHOD> method_rmap = {
    {"GET", GET},
    {"POST", POST},
    {"HEAD", HEAD},
    {"PUT", PUT},
    {"DELETE", DELETE},
    {"TRACE", TRACE},
    {"OPTIONS", OPTIONS},
    {"CONNECT", CONNECT},
    {"PATCH", PATCH}};

int main(int argc, char **argv)
{

    Parser p;
    p.add<int>("client", 'c', "client numbers", 256, cmdpaser::rangeof<int>(1, 65535));
    p.add<bool>("recv", 'r', "recv data", false);
    p.add<int>("time", 't', "run time(seconds)", 30, cmdpaser::rangeof<int>(1, 65535));
    p.add<string>("url", 'u', "url", [](const string &url)
                  { return url.size() < 1500; });
    p.add<string>("version", 'v', "http protocol version", "1.1", cmdpaser::oneof<string>({"1.0", "1.1", "2.0"}));
    p.add<string>("method", 'm', "method", "GET", cmdpaser::oneof<string>({"GET", "POST", "PUT", "DELETE", "HEAD", "OPTIONS", "PATCH"}));
    p.parse(argc, argv);

    auto clients = p.get<int>("client");
    auto recv = p.get<bool>("recv");
    auto run_time = p.get<int>("time");
    auto url = p.get<string>("url");
    auto protocol = p.get<string>("version");
    auto method_s = p.get<string>("method");

    string host, path;
    int port;
    bool is_https;
    if (!Request::parse_url(url, host, path, port, is_https))
    {
        std::cout << "parse url error" << std::endl;
        return 0;
    }
    std::cout << "host:" << host << "  path:" << path << "  port:" << port << "  is_https:" << (is_https ? "true" : "false") << std::endl;
    std::cout << "is recv:" << recv << std::endl;
    host = Request::host2ip(host);

    HTTP_METHOD method = method_rmap[method_s];
    HTTP_VERSION version = version_rmap[protocol];

    Request request(host, path, method, version);
    request.defaultHeader();

    string httpHeader = request.getRequest();

    int core_num = sysconf(_SC_NPROCESSORS_ONLN);
    assert(core_num > 0);
    int sock_num = clients / (core_num * 2);
    clients = core_num * 2;
    std::cout << "clients:" << clients << "  sock_num:" << sock_num << std::endl;
    Bench bench(host, port, clients, sock_num,run_time, recv, is_https);
    bench.loadRequest(httpHeader, "");
    bench.run();

}
