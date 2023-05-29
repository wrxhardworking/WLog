/**
 * @file    LogEvent.h
 * @brief   日志事件模块，包含了日志的基本属性。
 */
#pragma once

#include "LogLever.h"

#include <iostream>
#include <vector>
#include <memory>

namespace wlog {

    class Logger;

    //日志事件
    class LogEvent : public std::enable_shared_from_this<LogEvent> {
    public:
        using ptr = std::shared_ptr<LogEvent>;

        //初始化event
        LogEvent(const std::shared_ptr<Logger> &logger, LogLever::Lever lever, const char *filename, uint32_t line,
                 uint32_t elapse, uint32_t fiber, int32_t time);

        inline auto getFilename() const -> const char * { return m_filename_; }

        inline auto getLine() const -> uint32_t { return m_line_; }

        inline auto getElapse() const -> uint32_t { return m_elapse_; }

        inline auto getThread() const -> uint32_t { return m_threadId; }

        inline auto getFiber() const -> uint32_t { return m_fiberId; }

        inline auto getTime() const -> uint32_t { return m_time_; }

        inline auto getContent() const -> const std::string & { return m_content_; }

        inline auto getLogger() const -> std::shared_ptr<Logger> { return m_logger_; }

        inline auto getLever() const -> LogLever::Lever { return m_lever_; }

        /**
         * @brief  利用函数重载和模板的SFINAE(Substitution Failure Is Not An Error)特性,实现对不同类型参数字符串的拼接
         * @param  参数1：用户输入的数据
         * @return LogEvent & (继续迭代)
         */
        template<typename T>
        struct is_template : std::false_type {
        };
        template<template<typename...> class Template, typename... Args>
        struct is_template<Template<Args...>> : std::true_type {
        };
        template<typename T, typename = typename std::enable_if<(
                is_template<T>::value ||
                std::is_integral_v<T> ||
                std::is_floating_point_v<T>)>::type>
        LogEvent &operator<<(const T &contents) {
            if constexpr (std::is_same_v<T,std::string>){
                m_content_.append(contents);
                return *this;
            }
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
        inline LogEvent &operator<<(const void * p){
            m_content_.append(std::to_string(reinterpret_cast<uintptr_t>(p)));
            return *this;
        }
        inline LogEvent &operator<<(const char* content) {
            m_content_.append(content);
            return *this;
        }




    private:
        const char *m_filename_;                    //当前日志所在文件名
        uint32_t m_line_;                           //行号
        uint32_t m_elapse_;                         //程序启动到现在的毫秒数
        uint32_t m_threadId;                        //线程id
        uint32_t m_fiberId;                         //协程id(感觉现在我用不到)
        uint64_t m_time_;                           //时间戳
        std::string m_content_;                     //日志信息
        const std::shared_ptr<Logger> &m_logger_;   //日志器
        LogLever::Lever m_lever_;                   //日志等级
    };
}