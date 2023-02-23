
#ifndef BENCH_H
#define BENCH_H

#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include "noCopy.h"
#include "client.h"
#include "request.h"
#include "blockqueue.h"

namespace WebBench
{
    using std::string;

    struct ThreadData
    {
        ThreadData() : send_len_(0), recv_len_(0), success_conn_(0), faild_conn_(0), faild_send_(0), faild_recv_(0), success_IO_(0) {}
        ThreadData(size_t send_len, size_t recv_len, size_t success_conn, size_t faild_conn, size_t faild_send, size_t faild_recv, size_t success_IO)
            : send_len_(send_len), recv_len_(recv_len), success_conn_(success_conn),
              faild_conn_(faild_conn), faild_send_(faild_send), faild_recv_(faild_recv), success_IO_(success_IO)
        {
        }

        ThreadData &operator+(const ThreadData &rhs)
        {
            send_len_ += rhs.send_len_;
            recv_len_ += rhs.recv_len_;
            success_conn_ += rhs.success_conn_;
            faild_conn_ += rhs.faild_conn_;
            faild_send_ += rhs.faild_send_;
            faild_recv_ += rhs.faild_recv_;
            success_IO_ += rhs.success_IO_;
            return *this;
        }

        ThreadData &operator+=(const ThreadData &rhs)
        {
            send_len_ += rhs.send_len_;
            recv_len_ += rhs.recv_len_;
            success_conn_ += rhs.success_conn_;
            faild_conn_ += rhs.faild_conn_;
            faild_send_ += rhs.faild_send_;
            faild_recv_ += rhs.faild_recv_;
            success_IO_ += rhs.success_IO_;
            return *this;
        }

        size_t send_len_;
        size_t recv_len_;
        size_t success_conn_;
        size_t faild_conn_;
        size_t faild_send_;
        size_t faild_recv_;
        size_t success_IO_;
    };

    class Bench
    {
    public:
        Bench(const string &hostname, int port, int clients, int socknum, time_t seconds, bool recv, bool is_https);
        ~Bench() = default;
        DISABLE_COPY(Bench)

    public:
        void run();
        void showBench();

        void loadRequest(const std::string &request, const std::string &body);

        void alarm();

    private:
        string hostname_;
        int port_;
        int clients_;
        int socknum_;
        time_t seconds_;

        std::mutex mutex_;
        std::condition_variable cv_;

        bool recv_;
        bool is_https_;
        std::string request_;
        bool stop_;
        bool start_;

        ThreadData thread_res;
        BlcokQueue<ThreadData> data_queue_;
    };
} // namespace WebBench

#endif