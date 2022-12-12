#include <iostream>
#include"../Logger.h"
#include <chrono>
class Timer{
public:
    Timer(){
        m_curTime = std::chrono::high_resolution_clock::now();
    }
    ~Timer(){
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
    Logger::ptr logger(new Logger("my logger"));
    LogFormatter::ptr formatter(new LogFormatter("%c %d{%Y-%m-%d %H:%M:%S} %p 线程id:%t %f %l %m %n"));
    logger->setLevel(LogLever::FATAL);
    logger->setFormatter(formatter);

    {
        Timer timer;
        for(int i=0;i<1000000;++i){
            LOG_DEBUG(logger)<<"这是一条日志";
        }
    }
}