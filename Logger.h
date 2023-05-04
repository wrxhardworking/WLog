#pragma once

#include <type_traits>
#include <fstream>
#include <list>
#include <map>
#include <iostream>
#include <memory>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>
#include <mutex>

#include <pthread.h>
#include <cstdio>


#ifdef __linux__

#include <unistd.h>
#include <sys/syscall.h>

#elif _WIN32

#include<windows.h>

#endif


#ifdef __linux__

#define LOG_LEVER(logger, lever)\
    if(logger->getLevel()<=lever)\
        LoggerPackage(LogEvent::ptr(new LogEvent(logger,lever,__FILE__,__LINE__,0,syscall(SYS_gettid),0,time(0)))).getEvent()

#elif _WIN32

#define LOG_LEVER(logger,lever)\
    if(logger->getLevel()<=lever)\
        LoggerPackage(LogEvent::ptr(new LogEvent(logger,lever,__FILE__,__LINE__,0,GetCurrentThreadId(),0,time(0)))).getEvent()

#endif

#define GET_LOGMGR LoggerManger::getInstance()

#define GET_LOGGER GET_LOGMGR->getLogger()

#define LOG_INFO   LOG_LEVER(GET_LOGGER,LogLever::Lever::INFO)

#define LOG_DEBUG  LOG_LEVER(GET_LOGGER,LogLever::Lever::DEBUG)

#define LOG_ERROR  LOG_LEVER(GET_LOGGER,LogLever::Lever::ERROR)

#define LOG_FATAL  LOG_LEVER(GET_LOGGER,LogLever::Lever::FATAL)

#define LOG_WARN   LOG_LEVER(GET_LOGGER,LogLever::Lever::WARN)

#define LOG_ALERT  LOG_LEVER(GET_LOGGER,LogLever::Lever::ALERT)

#define LOG_NOTICE LOG_LEVER(GET_LOGGER,LogLever::Lever::NOTICE)

#define LOG_NOTSET LOG_LEVER(GET_LOGGER,LogLever::Lever::NOTSET)


class Logger;

class LogLever;

//日志级别
class LogLever {
public:
    enum Lever {
        FATAL = 0,
        /// 高优先级情况，例如数据库系统崩溃
        ALERT = 100,
        /// 严重错误，例如硬盘错误
        CRIT = 200,
        /// 错误
        ERROR = 300,
        /// 警告
        WARN = 400,
        /// 正常但值得注意
        NOTICE = 500,
        /// 一般信息
        INFO = 600,
        /// 调试信息
        DEBUG = 700,
        /// 未设置
        NOTSET = 800,
    };

    static const char *Tostring(LogLever::Lever lever_);
};

//日志事件
class LogEvent : public std::enable_shared_from_this<LogEvent> {
public:
    using ptr = std::shared_ptr<LogEvent>;

    //初始化event
    LogEvent(std::shared_ptr<Logger> logger, LogLever::Lever lever, std::string_view filename, uint32_t line,
             uint32_t elapse, uint32_t threadid, uint32_t fiber, int32_t time);

    //得到相应的数值
    auto getfilename() const -> const char * { return m_filename_; }

    auto getline() const -> int32_t { return m_line_; }

    auto getelape() const -> uint32_t { return m_elapse_; }

    auto getthread() const -> uint32_t { return m_threadid_; }

    auto getfiber() const -> uint32_t { return m_fiberid_; }

    auto gettime() const -> uint32_t { return m_time_; }

    auto getcontent() const -> const std::string & { return m_content_; }//这是日志信息

    auto getlogger() const -> std::shared_ptr<Logger> { return m_logger_; }

    auto getLever() const -> LogLever::Lever { return m_lever_; }

    template<typename T>
    struct is_template : std::false_type {
    };
    template<template<typename...> class Template, typename... Args>
    struct is_template<Template<Args...>> : std::true_type {
    };

    template<typename T, typename = typename std::enable_if<!std::is_same_v<T, std::string> && (
            is_template<T>::value ||
            std::is_integral_v<T> ||
            std::is_floating_point_v<T>)>::type>
    LogEvent &operator<<(const T &contents) {
        if constexpr (std::is_integral_v<T> || std::is_floating_point_v<T>) {
            m_content_.append(std::to_string(contents));
            return *this;
        }

        if constexpr (is_template<T>::value) {
            using TYPE = typename T::value_type;
            if constexpr (std::is_same_v<T, std::vector<TYPE>>) {
                if constexpr (std::is_integral_v<TYPE> ||
                              std::is_floating_point_v<TYPE>) {
                    for (auto &&content: contents) {
                        m_content_.append(std::to_string(content));
                    }
                    return *this;
                } else if (std::is_same_v<TYPE, std::string> ||
                           std::is_same_v<TYPE, const std::string> ||
                           std::is_same_v<TYPE, const char *> ||
                           std::is_same_v<TYPE, char *>) {
                    for (auto &&content: contents) {
                        m_content_.append(content);
                    }
                    return *this;
                }
            }
        }
        return *this;
    }

    LogEvent &operator<<(std::string_view content) {
        m_content_.append(content);
        return *this;
    }

private:
    const char *m_filename_ = nullptr; //当前日志所在文件名
    int32_t m_line_ = 0;               //行号
    uint32_t m_elapse_ = 0;            //程序启动到现在的毫秒数
    uint32_t m_threadid_ = 0;          //线程id
    uint32_t m_fiberid_ = 0;           //协程id(感觉现在我用不到)
    uint64_t m_time_ = 0;              //时间戳
    std::string m_content_;            //日志信息
    std::shared_ptr<Logger> m_logger_; //日志器
    LogLever::Lever m_lever_;          //日志等级

};

class LogFormatter {
public:
    using ptr = std::shared_ptr<LogFormatter>;

    explicit LogFormatter(const std::string &pattern);

    auto format(std::shared_ptr<Logger> logger, LogLever::Lever lever, LogEvent::ptr event) -> std::string;

    //初始化，解析日志模板
    enum class Parse {
        NORMAL = 0,
        TEMPLATE
    };

    const std::string &getPattern() {
        return m_pattern_;
    }

    void init();

public:
    //支持日志格式的可扩展性
    class FormatterItem {
    public:
        using ptr = std::shared_ptr<FormatterItem>;

        //fixme add logger
        virtual void
        format(std::string &log, std::shared_ptr<Logger> logger, LogLever::Lever lever, LogEvent::ptr event) = 0;
    };

private:
    //日志格式模板
    std::string m_pattern_; //模式
    //日志解析后的格式
    std::vector<FormatterItem::ptr> m_items_;
};

//日志器
class Logger : public std::enable_shared_from_this<Logger> {
public:
    using ptr = std::shared_ptr<Logger>;

    explicit Logger(std::string_view name = "root");

    void log(LogLever::Lever lever_, LogEvent::ptr event);

    //多线程下要加锁
    void setLevel(LogLever::Lever val) {
        std::lock_guard<std::mutex> locker(Mutex);
        m_lever_ = val;
    }

    void setName(std::string_view new_name) {
        std::lock_guard<std::mutex> locker(Mutex);
        m_name_ = new_name;
    }

    LogLever::Lever getLevel() const { return m_lever_; }

    //得到日志名称
    const std::string &getname() const { return m_name_; }

    //设置日志格式器
    //多线程下加锁
    void setFormatter(LogFormatter::ptr formatter) {
        std::lock_guard<std::mutex> locker(Mutex);
        m_formatter_ = std::move(formatter);
    }

    inline LogFormatter::ptr getFormatter() const { return m_formatter_; }

private:
    std::mutex Mutex;
    LogFormatter::ptr m_formatter_;
    std::string m_name_;                        //日志名称
    LogLever::Lever m_lever_;                 //日志级别
};

//日志包装器
class LoggerPackage {
public:
    explicit LoggerPackage(LogEvent::ptr event);

    ~LoggerPackage();

    auto getEvent() const -> LogEvent & { return *m_event_; }

private:
//事件
    LogEvent::ptr m_event_;
};

class LoggerManger {
public:
    LoggerManger() : root(new Logger()) {
        LogFormatter::ptr formatter(new LogFormatter("%c %d{%Y-%m-%d %H:%M:%S} %p %t %f %l %m %n"));
        root->setLevel(LogLever::INFO);
        root->setFormatter(formatter);
    }

    static LoggerManger *getInstance() {
        static LoggerManger loggerManger;
        return &loggerManger;
    }

    Logger::ptr getLogger() {
        return root;
    }

private:
    Logger::ptr root;
};

