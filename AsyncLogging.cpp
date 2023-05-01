//
// Created by 28108 on 2022/12/4.
//

#include "AsyncLogging.h"

#include <functional>
#include <memory>
#include <iostream>

AsyncLogging::AsyncLogging(int flushTimeInterval) : flushTimeInterval(flushTimeInterval), running(true),
                                                    Mutex(), condition(), currentBuffer(new Buffer),
                                                    nextBuffer(new Buffer), buffers() {
    //缓冲区的初始化
    currentBuffer->bezero();
    nextBuffer->bezero();

    //获取路径
    char *buffer;
    if ((buffer = getcwd(nullptr, 0)) == nullptr) {
        perror("get cwd error");
    }
    bzero(base, 255);
    sprintf(base, "../../%s.log", basename(buffer));

    //预留了空间
    //下面临时对象的write buffers 也是预留了16
    buffers.reserve(16);
    WorkThread = (std::move(std::thread([this] { backendThread(); })));
}

void AsyncLogging::append(std::string_view log, int len) {
    {
        std::lock_guard<std::mutex> locker(Mutex);
        //当前缓冲区容量充足 直接写入缓冲区
        if (currentBuffer->avail() > len) {
            currentBuffer->append(log.data(), len);
        }
            //当前缓冲区的可用吧容量不足 则说明前端写满了 要预备使用buffer
        else {
            //将前端日志写入消费者队列中 通知消费者消费
            buffers.push_back(std::move(currentBuffer));
            //如果后端buffer未启用
            if (nextBuffer) {
                currentBuffer = std::move(nextBuffer);
            }
                //说明日志写的很频繁 这个时候就要重新开辟一块buffer 这种情况十分罕见
            else {
                currentBuffer = std::make_unique<Buffer>();
            }
            currentBuffer->append(log.data(), len);
            condition.notify_all();
        }
    }
}

void AsyncLogging::backendThread() {
    //以追加的方式打开一个文件
    FILE *fp = fopen(base, "a+");
    //以下两块buffer是为后面的更替做准备
    BufferPtr replaceBuffer1(new Buffer);
    BufferPtr replaceBuffer2(new Buffer);
    //初始化
    replaceBuffer1->bezero();
    replaceBuffer2->bezero();
    //输出到文件中buffer 用一个容器装起
    Buffers BufferToWrite;
    //预留空间 省去打日志时申请时间
    BufferToWrite.reserve(16);
    while (running) {
        //以下是临界区
        {
            std::unique_lock<std::mutex> locker(Mutex);
            //fixme 会不会存在虚假唤醒? buffers不为空或者等待事件超过三秒
            if (buffers.empty()) {
                condition.wait_for(locker, std::chrono::seconds(flushTimeInterval));
            }
            //超时阶段
            buffers.push_back(std::move(currentBuffer));
            currentBuffer = std::move(replaceBuffer1);
            BufferToWrite.swap(buffers);
            //如果预备buffer被占用
            if (!nextBuffer) {
                nextBuffer = std::move(replaceBuffer2);
            }
        }

        for (const auto &buffer: BufferToWrite) {
            //fixme 利用宏定义区分操作系统
            fwrite(buffer->data(), 1, buffer->length(), fp);
        }

        if (BufferToWrite.size() > 2) {
            BufferToWrite.resize(2);
        }
        if (!replaceBuffer1) {
            replaceBuffer1 = std::move(BufferToWrite.back());
            BufferToWrite.pop_back();
        }
        if (!replaceBuffer2) {
            replaceBuffer2 = std::move(BufferToWrite.back());
            BufferToWrite.pop_back();
        }
        //将写缓冲区置空
        BufferToWrite.clear();
        // fixme flush
        fflush(fp);
    }
    //fixme flush
    fflush(fp);

    if (fp) {
        fclose(fp);
    }
}

AsyncLogging *AsyncLogging::getInstance() {
    static AsyncLogging asyncLogging;
    return &asyncLogging;
}
