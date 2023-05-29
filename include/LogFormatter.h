/**
 * @file    LogFormatter.h
 * @brief   日志输出格式模块，包含了日志格式的初始化及多态实现的日志属性拼装
 */


#pragma once

#include "LogEvent.h"

#include <iostream>
#include <string>

namespace wlog {

    class LogFormatter {
    public:
        using ptr = std::shared_ptr<LogFormatter>;

        explicit LogFormatter(std::string_view pattern);

        void format(const std::shared_ptr<Logger> &logger, LogLever::Lever lever, const LogEvent::ptr &event,
                    std::string &log);

        //初始化，解析日志模板
        enum class Parse {
            NORMAL = 0,
            TEMPLATE
        };

        [[maybe_unused]] const std::string &getPattern() {
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
            format(std::string &log, const std::shared_ptr<Logger> &logger, LogLever::Lever lever,
                   const LogEvent::ptr &event) = 0;
        };

    private:
        //日志格式模板
        std::string m_pattern_;
        //日志解析后的格式
        std::vector<FormatterItem::ptr> m_items_;
    };
}
