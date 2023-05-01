//
// Created by 28108 on 2022/12/4.
//

#pragma once


#include "FixBuffer.hpp"
#include "Logger.h"

#include<condition_variable>
#include <vector>
#include <atomic>
#include <memory>
#include <mutex>
#include <thread>
#include <chrono>


class AsyncLogging {
public:

    static AsyncLogging *getInstance();

    ~AsyncLogging() { if (running) stop(); }

    //前端向buffer添加日志
    void append(std::string_view log, int len);

    void stop() {
        running = false;
        condition.notify_all();
        WorkThread.join();
    }

    using Buffer = FixBuffer<LargeBuffer>;
    using BufferPtr = std::unique_ptr<Buffer>;
    using Buffers = std::vector<BufferPtr>;
private:
    //作为单例 防止外界构造对象
    char base[255]{};

    explicit AsyncLogging(int flushTimeInterval = 3);
    //消费者线程跑的函数
    void backendThread();
    std::atomic<bool> running;
    //后端工作线程 暂时未初始化
    std::thread WorkThread;
    std::mutex Mutex;
    int flushTimeInterval;
    std::condition_variable condition;
    BufferPtr currentBuffer;
    BufferPtr nextBuffer;
    Buffers buffers;
};