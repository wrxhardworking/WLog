/**
 * @file    AsynLog.h
 * @brief   日志异步输出到文件的模块。
 */

#pragma once

#include "FixBuffer.h"
#include "Logger.h"

#include <condition_variable>
#include <atomic>
#include <thread>

namespace wlog {

    class AsyncLog : public Singleton<AsyncLog> {

    public:
        AsyncLog();

        inline ~AsyncLog() { if (running_) stop(); }

        //前端向buffer添加日志
        void append(std::string_view log, int len);

        inline void stop() {
            running_ = false;
            condition_.notify_all();
            workThread_.join();
        }

        using Buffer = FixBuffer<LargeBuffer>;
        using BufferPtr = std::unique_ptr<Buffer>;
        using Buffers = std::vector<BufferPtr>;
    private:
        //作为单例 防止外界构造对象
        //消费者线程跑的函数
        void backendThread();

        char base_[255]{};
        std::atomic<bool> running_;
        //后端工作线程
        std::thread workThread_;
        std::mutex mutex_;
        int flushTimeInterval_;
        std::condition_variable condition_;
        BufferPtr currentBuffer_;
        BufferPtr nextBuffer_;
        Buffers buffers_;
    };
}