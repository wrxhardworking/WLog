/**
 * @file    Singleton.h
 * @brief   一个常见单例的模板类，依赖c++的静态局部性，减少代码的冗余。
 */

#pragma once

namespace wlog {

    template<typename T>
    class Singleton {
    public:
        static T *getInstance() {
            static T t;
            return &t;
        }
    };
}

