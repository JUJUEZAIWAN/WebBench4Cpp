/*
 * @Author: 欢乐水牛
 * @Date: 2023-02-20 19:31:43
 * @LastEditTime: 2023-02-23 14:40:41
 * @LastEditors: 欢乐水牛
 * @Description:
 * @FilePath: /CPP/WebBench4Cpp/src/bench.cpp
 *
 */

#include <iostream>
#include <cassert>
#include "bench.h"
using namespace WebBench;


Bench::Bench(const string &hostname, int port, int clients, int socknum, time_t seconds, bool recv, bool is_https = false)
    : hostname_(hostname), port_(port), clients_(clients), socknum_(socknum), seconds_(seconds), recv_(recv),
      is_https_(is_https), stop_(false), start_(false), thread_res()
{
    assert(clients_ > 0);
    assert(socknum_ > 0);
    assert(seconds_ > 10);
}

void Bench::loadRequest(const std::string &request, const std::string &body = "")
{
    request_ = request + body;
}

void Bench::run()
{
    std::vector<std::thread> threads;
    threads.reserve(clients_);

    for (int i = 0; i < clients_; i++)
    {
        threads.emplace_back(
            [this]()
            {
                {
                    std::unique_lock<std::mutex> lock(mutex_);

                    cv_.wait(lock, [this]() -> bool
                             { return start_; }); // wait for start , all threads will start at the same time
                }

                Client client(socknum_, is_https_);

                // test connect ,if connect faild , the thread will  faild when reconnect,so return
                bool is_connect = client.connect(hostname_, port_);

                auto conn_num = client.getSocketNum();
                if (!is_connect)
                {
                    data_queue_.push({0, 0, conn_num, socknum_ - conn_num, 0, 0,0});
                    return;
                }
                else
                {
                    data_queue_.push({0, 0, conn_num, socknum_ - conn_num, 0, 0,0});
                }

                while (1)
                {
                    if (stop_)
                        return;
                    client.send(request_, stop_);
                    if (stop_)
                        return;
                    if (recv_)
                    {
                        client.recv(stop_);
                        data_queue_.push({client.getSendLen(), client.getRecvLen(), 0, 0, client.getSocketSendNumError(), client.getSocketRecvNumError(),client.getSocketNum()});
                    }
                    else
                    {
                        data_queue_.push({client.getSendLen(), 0, 0, 0, client.getSocketSendNumError(), client.getSocketRecvNumError(),client.getSocketNum()});
                    }

                }
            });
    }

    std::cout << "start bench:" << std::endl;
    auto now = std::chrono::system_clock::now();
    time_t tt = std::chrono::system_clock::to_time_t(now);
    std::cout << "begin time:" << ctime(&tt) << std::endl;
    alarm(); // start alarm,no block

    while (!stop_)
    {
        while (!data_queue_.empty())
        {
            auto thread_data = data_queue_.pop();
            thread_res += thread_data;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    now = std::chrono::system_clock::now();
    tt = std::chrono::system_clock::to_time_t(now);
    std::cout << "end time:" << ctime(&tt) << std::endl;

    std::cout << "thread size:" << threads.size() << std::endl;
    std::cout << "stop" << std::endl;
    showBench();

    for (auto &thread : threads)
    {
        thread.detach();
    }
}

// TODO： format bench result
void Bench::showBench()
{
    std::cout << "total send len:" << thread_res.send_len_ * 1.0 / (1000 * 1000) << " Mb   send speed:" << thread_res.send_len_ * 1.0 / (1000 * 1000) / seconds_ << "mb/s" << std::endl;
    std::cout << "total recv len:" << thread_res.recv_len_ * 1.0 / (1000 * 1000) << " Mb   recv speed:" << thread_res.recv_len_ * 1.0 / (1000 * 1000) / seconds_ << "mb/s" << std::endl;
    std::cout << "faild connection:" << thread_res.faild_conn_ << std::endl;
    std::cout << "success connection:" << thread_res.success_conn_ << std::endl;
    std::cout << "total connection:" << thread_res.success_conn_ + thread_res.faild_conn_ << std::endl;
    std::cout << "faild send count:" << thread_res.faild_send_ << " pages" << std::endl;
    std::cout << "faild recv count:" << thread_res.faild_recv_ << " pages" << std::endl;
    std::cout << "total error IO count:" << thread_res.faild_recv_ + thread_res.faild_send_ << " pages" << std::endl;
    std::cout << "IO speed:" << thread_res.success_IO_ * 1.0 / seconds_ << " pages/s" << std::endl;
    std::cout << "total IO count:" << thread_res.success_IO_ << " pages" << std::endl;

}

void Bench::alarm()
{
    std::thread t([this]()
                  {
            std::this_thread::sleep_for(std::chrono::seconds(seconds_));
            stop_ = true; });
    // cv will notify all thread , so all thread will start at the same time
    start_ = true;
    cv_.notify_all();
    t.detach();
}