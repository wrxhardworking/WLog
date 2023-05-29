#include "../include/Wlog.h"

#include <mutex>
#include <chrono>
#include <thread>

//定时器 用于测试性能
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
        printf("%ld us (%lf ms)\n", duration, ms);
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
     * part III:
     * 使用方式
     * part IV:
     * 线程安全的测试
    */

#if 1
    //除去第一条初始化的时间
    WLOG_LOG_DEBUG << "";
    WLOG_GET_LOGGER->setIsStdout(false);
    {
        for (int j = 0; j < 5; ++j) {
            Timer timer;
            {
                for (int i = 0; i < 1000000; ++i) {
                    WLOG_LOG_DEBUG << "hello world";
                }
            }
        }
    }

#endif
#if 0
    //设置日志等级
    WLOG_GET_LOGGER->setLevel(LogLever::ALERT);
    //设置日志名称
    WLOG_GET_LOGGER->setName("service");
    //设置日志格式
    WLOG_GET_LOGGER->setFormatter(".....");
    //设置输出地限制
    WLOG_GET_LOGGER->setIsFile(false);
    WLOG_GET_LOGGER->setIsStdout(false);
    //......敬请期待(指定日志输出地 默认名字是target_name 文件的默认路径是../build(构建目录))
#endif
#if 0
    /*
    * 1.支持普通字符串类型
    * 2.支持std::string类型
    * 3.支持std::vector<int>,std::vector<std::string>等类型
    * 4.支持打印地址(十进制进行打印)
    */
    int a = 100;
    std::string log = "hello world ";
    std::vector<const char *> s{"hello"};
    WLOG_LOG_INFO<<log<<s;
    WLOG_LOG_INFO<<"HELLO WORLD";
    WLOG_LOG_INFO<<std::vector<int>{1,2,3,4,5};
    WLOG_LOG_INFO<<1.1111111;
    WLOG_LOG_INFO<<111111111;
    WLOG_LOG_INFO<<&a;
#endif
#if 0
    std::vector<std::thread> v;
    v.reserve(8);
    for (int i = 0; i < 8; i++) {
        v.emplace_back([=]() {
            WLOG_GET_LOGGER->setName("test_logger_" + std::to_string(i));
            WLOG_LOG_INFO << "hello world";
        });
    }
    for (auto &x: v) {
        x.join();
    }
#endif
}