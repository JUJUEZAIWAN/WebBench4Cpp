


#ifndef BLOCKQUEUE_H
#define BLOCKQUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include "noCopy.h"

namespace WebBench
{

    template <typename T>
    class BlcokQueue
    {
    public:
        BlcokQueue() = default;
        ~BlcokQueue() = default;
        DISABLE_COPY(BlcokQueue)

    public:
        void push(const T &data)
        {
            std::unique_lock<std::mutex> lock(mutex_);
            queue_.push(data);
            cv_.notify_one();
        }

        void push(T &&data)
        {
            std::unique_lock<std::mutex> lock(mutex_);
            queue_.push(std::move(data));
            cv_.notify_one();
        }

        T pop()
        {
            std::unique_lock<std::mutex> lock(mutex_);
            while (queue_.empty())
            {
                cv_.wait(lock);
            }
            T data = std::move(queue_.front());
            queue_.pop();
            return data;
        }

        bool empty()
        {
            std::unique_lock<std::mutex> lock(mutex_);
            return queue_.empty();
        }

        size_t size()
        {
            std::unique_lock<std::mutex> lock(mutex_);
            return queue_.size();
        }

    private:
        std::queue<T> queue_;
        std::mutex mutex_;
        std::condition_variable cv_;
    };

}
#endif // BLOCKQUEUE_H