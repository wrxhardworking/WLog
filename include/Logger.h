/**
 * @file    Logger.h
 * @brief   日志器模块，这个模块是整个项目的中心处理器，包含了与用户和日志系统交互的方法。
 */

#pragma once

#include "Singleton.h"
#include "LogFormatter.h"

#include <mutex>
#include <atomic>

namespace wlog {
    //日志器
    class Logger : public std::enable_shared_from_this<Logger> {
    public:

        using ptr = std::shared_ptr<Logger>;

        explicit Logger(const char * = "root");

        void log(LogLever::Lever lever_, const LogEvent::ptr &event);

        inline void setLevel(LogLever::Lever val) {
            m_lever_ = val;
        }

        inline void setName(std::string_view new_name) {
            m_name_ = new_name.data();
        }

        //多线程下加锁(以防万一 ）当然还有别的办法：1.设置为nocopyable 2.用unique_ptr进行维护
        inline void setFormatter(std::string_view pattern) {
            m_formatter_.reset(new LogFormatter(pattern));
        }

        inline void setIsStdout(bool on) {
            isStdout = on;
        }

        inline void setIsFile(bool on) {
            isFile = on;
        }

        inline LogLever::Lever getLevel() const { return m_lever_; }

        inline const char *getName() const { return m_name_; }

        [[maybe_unused]] inline LogFormatter::ptr getFormatter() const { return m_formatter_; }

    private:
        bool isStdout;
        bool isFile;

        LogFormatter::ptr m_formatter_;
        const char *m_name_;
        LogLever::Lever m_lever_;
    };

    //日志包装器
    class LoggerPackage {
    public:
        explicit LoggerPackage(LogEvent::ptr &&event);

        ~LoggerPackage();

        [[nodiscard]]inline auto getEvent() const -> LogEvent & { return *m_event_; }

    private:
        //事件
        LogEvent::ptr m_event_;
    };


    //这是线程的局部变量 每一个线程维护一个日志器
    namespace {
        thread_local Logger::ptr logger_ = nullptr;
    }

    class LoggerManger : public Singleton<LoggerManger> {
    public:
        inline Logger::ptr &getLogger() {
            if (!logger_) {
                logger_ = std::make_shared<Logger>();
            }
            return logger_;
        }
    };
}