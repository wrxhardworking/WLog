/**
 * @file    FixBuffer.h
 * @brief   这是缓冲区用到自定义的buffer模块,前后统一。
 *
 */

#pragma once

#include <cstring>

namespace wlog {
    //缓冲区buffer的大小设定
    const int LargeBuffer = 4000 * 1000;   //4MB
    [[maybe_unused]] const int SmallBuffer = 4000;

    //非类型参数的模板类 来指定缓冲buffer的大小
    template<int SIZE>
    class FixBuffer {
    public:

        inline FixBuffer() : cur_(data_) {}

        ~FixBuffer() = default;

        [[nodiscard]]inline const char *data() const { return data_; }

        [[nodiscard]]inline int length() const { return static_cast<int>(cur_ - data_); }

        [[maybe_unused]]inline char *current() { return cur_; }

        [[nodiscard]]inline int avail() const { return static_cast<int>(end() - cur_); }

        inline void append(const char *buf, size_t len) {
            if (static_cast<size_t>(avail()) > len) {
                //给字符地址初始化
                memcpy(cur_, buf, len);
                cur_ = cur_ + len;
            }
        }

        [[maybe_unused]]inline void reset() { cur_ = data_; }

        inline void beZero() { bzero(data_, sizeof(data_)); }

    private:

        [[nodiscard]]inline const char *end() const { return data_ + sizeof data_; }

        char data_[SIZE]{};

        char *cur_;
    };
}