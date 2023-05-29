#include "../include/AsyncLog.h"

#include <unistd.h>
#include <functional>
#include <memory>
#include <iostream>
#include <cassert>

using namespace wlog;

AsyncLog::AsyncLog() : flushTimeInterval_(FLUSH_TIME), running_(true),
                       mutex_(), condition_(), currentBuffer_(new Buffer),
                       nextBuffer_(new Buffer), buffers_() {
    //缓冲区的初始化
    currentBuffer_->beZero();
    nextBuffer_->beZero();

    //获取路径
    char *buffer;
    if ((buffer = getcwd(nullptr, 0)) == nullptr) {
        perror("get cwd error");
    }
    bzero(base_, 255);
    sprintf(base_, "../../%s.log", basename(buffer));

    //预留了空间
    //下面临时对象的write buffers_ 也是预留了16
    buffers_.reserve(16);
    workThread_ = (std::move(std::thread([this] { backendThread(); })));
}

void AsyncLog::append(std::string_view log, int len) {
    {
        std::lock_guard<std::mutex> locker(mutex_);
        //当前缓冲区容量充足 直接写入缓冲区
        if (currentBuffer_->avail() > len) {
            currentBuffer_->append(log.data(), len);
        }
            //当前缓冲区的可用吧容量不足 则说明前端写满了 要预备使用buffer
        else {
            //将前端日志写入消费者队列中 通知消费者消费
            buffers_.push_back(std::move(currentBuffer_));
            //如果后端buffer未启用
            if (nextBuffer_) {
                currentBuffer_ = std::move(nextBuffer_);
            }
                //说明日志写的很频繁 这个时候就要重新开辟一块buffer 这种情况十分罕见
            else {
                currentBuffer_ = std::make_unique<Buffer>();
            }
            currentBuffer_->append(log.data(), len);
            condition_.notify_all();
        }
    }
}

void AsyncLog::backendThread() {
    //以追加的方式打开一个文件
    FILE *fp = fopen(base_, "a+");
    //以下两块buffer是为后面的更替做准备
    BufferPtr replaceBuffer1(new Buffer);
    BufferPtr replaceBuffer2(new Buffer);
    //初始化
    replaceBuffer1->beZero();
    replaceBuffer2->beZero();
    //输出到文件中buffer 用一个容器装起
    Buffers BufferToWrite;
    //预留空间 省去打日志时申请时间
    BufferToWrite.reserve(16);
    while (running_) {
        //以下是临界区
        {
            std::unique_lock<std::mutex> locker(mutex_);
            //fixme 会不会存在虚假唤醒? buffers不为空或者等待事件超过三秒
            if (buffers_.empty()) {
                condition_.wait_for(locker, std::chrono::seconds(flushTimeInterval_));
            }
            //超时阶段
            buffers_.push_back(std::move(currentBuffer_));
            currentBuffer_ = std::move(replaceBuffer1);
            BufferToWrite.swap(buffers_);
            //如果预备buffer被占用
            if (!nextBuffer_) {
                nextBuffer_ = std::move(replaceBuffer2);
            }
        }

        assert(!BufferToWrite.empty());

        if (BufferToWrite.size() > 25)
        {
            char buf[256];
            fputs(buf, stderr);
            BufferToWrite.erase(BufferToWrite.begin()+2, BufferToWrite.end());
        }


        for (const auto &buffer: BufferToWrite) {
            //fixme 利用宏定义区分操作系统
            fwrite(buffer->data(), 1, buffer->length(), fp);
        }

        if (BufferToWrite.size() > 2) {
            BufferToWrite.resize(2);
        }
        if (!replaceBuffer1) {
            assert(!BufferToWrite.empty());
            replaceBuffer1 = std::move(BufferToWrite.back());
            BufferToWrite.pop_back();
            replaceBuffer1->reset();
        }
        if (!replaceBuffer2) {
            assert(!BufferToWrite.empty());
            replaceBuffer2 = std::move(BufferToWrite.back());
            BufferToWrite.pop_back();
            replaceBuffer2->reset();
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


