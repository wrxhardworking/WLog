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
    //此处可以设置回滚大小和刷新时间 不能干等前端把缓冲区填满自己不做事情
    static AsyncLogging *getInstance();

    ~AsyncLogging() { if (running) stop(); }

    void append(std::string_view log, int len);//前端写日志向缓冲buffer中增加的部分

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

    explicit AsyncLogging(int flushTimeInterval = 3);//刷新间隔
    void backendThread();//消费者线程跑的函数
    std::atomic<bool> running;//原子变量
    std::thread WorkThread;//后端工作线程 暂时未初始化
    std::mutex Mutex;//锁
    int flushTimeInterval;
    std::condition_variable condition;//条件变量
    BufferPtr currentBuffer;
    BufferPtr nextBuffer;
    Buffers buffers;//缓冲区
};