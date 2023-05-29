#include "../include/AsyncLog.h"

using namespace wlog;

Logger::Logger(const char *name) : m_name_(name), m_lever_(LogLever::DEBUG),
                                   isStdout(true), isFile(true),
                                   m_formatter_(new LogFormatter("%c %d{%Y-%m-%d %H:%M:%S} %p %t %f %l %m %n")) {
}

void Logger::log(LogLever::Lever lever_, const LogEvent::ptr &event) {

    std::string log;

    m_formatter_->format(shared_from_this(), lever_, event, log);
    //如果接收到的为空 报错也要哦将错误信息输出到日志文件中
    if (log.empty()) {
        log += "file appender error";
    }
    //fwrite再linux下是线程安全的  fixme
    if (isStdout)
        fwrite(log.data(), 1, log.length(), stdout);

    //向缓冲区传递
    if (isFile)
        AsyncLog::getInstance()->append(log, static_cast<int>(log.length()));
}

//创建一个临时对象 利用raii避免了一些不必要的线程安全问题
LoggerPackage::LoggerPackage(LogEvent::ptr &&event) : m_event_(std::move(event)) {
}

LoggerPackage::~LoggerPackage() {
    m_event_->getLogger()->log(m_event_->getLever(), m_event_);
}