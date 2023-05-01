#include <iostream>
#include "../Logger.h"
#include <chrono>
#include <memory>

class Timer {
public:
    Timer() {
        m_curTime = std::chrono::high_resolution_clock::now();
    }

    ~Timer() {
        auto end = std::chrono::high_resolution_clock::now();
        auto startTime = std::chrono::time_point_cast<std::chrono::microseconds>(m_curTime)
                .time_since_epoch().count();
        auto endTime = std::chrono::time_point_cast<std::chrono::microseconds>(end)
                .time_since_epoch().count();
        auto duration = endTime - startTime;
        double ms = duration * 0.001;//得到毫秒
        printf("%lld us (%lf ms)\n", duration, ms);
        fflush(stdout);
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_curTime;
};

int main() {
    /*
     * part I:
     * 测试性能
     * part II:
     * 自定义日志名称，日志格式
    */
#if 0
    {
        Timer timer;
        {
            for (int i = 0; i < 10000; ++i) {
                LOG_INFO << "hello world";
            }
        }
    }

#endif
    //设置日志等级
    GET_LOGGER->setLevel(LogLever::ALERT);
    //设置日志名称
    GET_LOGGER->setName("service");
    //设置日志格式
    GET_LOGGER->setFormatter(std::make_shared<LogFormatter>("....."));
    //......敬请期待(指定日志输出地 默认名字是target_name 文件的默认路径是../build(构建目录))
}