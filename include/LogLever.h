/**
 * @file    LogLever.h
 * @brief   日志等级模块，常见的日志等级，具体参照syslog
 */


#pragma once

namespace wlog {
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
        };

        static const char *Tostring(LogLever::Lever lever_);
    };
}
