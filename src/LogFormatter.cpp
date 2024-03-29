#include "../include/LogFormatter.h"
#include "../include/Logger.h"

#include <functional>
#include <map>
#include <utility>

using namespace wlog;

LogFormatter::LogFormatter(std::string_view pattern) : m_pattern_(pattern) {
    init();
}

// fixme 以下是日志格式 可以扩展 不断增加日志的格式
//字符型时打印出来
void LogFormatter::format(const std::shared_ptr<Logger> &logger, LogLever::Lever lever,
                          const LogEvent::ptr &event, std::string &log) {
    for (auto &i: m_items_) {
        i->format(log, logger, lever, event);
    }
}

//以下的构造函数是为了统一格式
class MessageFormatItem : public LogFormatter::FormatterItem {
public:
    explicit MessageFormatItem(const std::string &str = "") {
    }

    void format(std::string &log, const std::shared_ptr<Logger> &logger, LogLever::Lever lever,
                const LogEvent::ptr &event) override {
        log.append(event->getContent());
    }
};

class LeverFormatItem : public LogFormatter::FormatterItem {
public:
    explicit LeverFormatItem(const std::string &str = "") {
    }

    void format(std::string &log, const std::shared_ptr<Logger> &logger, LogLever::Lever lever,
                const LogEvent::ptr &event) override {
        log.append(LogLever::Tostring(lever));
    }
};

class ElapseFormatItem : public LogFormatter::FormatterItem {
public:
    explicit ElapseFormatItem(const std::string &str = "") {
    }

    void format(std::string &log, const std::shared_ptr<Logger> &logger, LogLever::Lever lever,
                const LogEvent::ptr &event) override {
        //fixme
        log.append(std::to_string(event->getElapse()));
    }
};

class NameFormatItem : public LogFormatter::FormatterItem {
public:
    explicit NameFormatItem(const std::string &str = "") {
    }

    void format(std::string &log, const std::shared_ptr<Logger> &logger, LogLever::Lever lever,
                const LogEvent::ptr &event) override {
        log.append(logger->getName());
    }
};

class ThreadIdFormatItem : public LogFormatter::FormatterItem {
public:
    explicit ThreadIdFormatItem(const std::string &str = "") {
    }

    void format(std::string &log, const std::shared_ptr<Logger> &logger, LogLever::Lever lever,
                const LogEvent::ptr &event) override {
        log.append(std::to_string(event->getThread()));
    }
};

class FiberIdFormatItem : public LogFormatter::FormatterItem {
public:
    explicit FiberIdFormatItem(const std::string &str = "") {
    }

    void format(std::string &log, const std::shared_ptr<Logger> &logger, LogLever::Lever lever,
                const LogEvent::ptr &event) override {
        log.append(std::to_string(event->getFiber()));
    }
};

//fixme
class DateTimeFormatItem : public LogFormatter::FormatterItem {
public:
    //常量引用可以直接赋值常量
    explicit DateTimeFormatItem(std::string format = "%Y-%M-%d %H:%M:%s") : m_format(std::move(format)) {
    }

    void format(std::string &log, const std::shared_ptr<Logger> &logger, LogLever::Lever lever,
                const LogEvent::ptr &event) override {
        struct tm tm{};
        time_t time = event->getTime();
        localtime_r(&time, &tm);
        char buf[64];
        strftime(buf, sizeof(buf), m_format.c_str(), &tm);
        log.append(buf);
    }

private:
    std::string m_format;
};

class FilenameFormatItem : public LogFormatter::FormatterItem {
public:
    explicit FilenameFormatItem(const std::string &str = "") {
    }

    void format(std::string &log, const std::shared_ptr<Logger> &logger, LogLever::Lever lever,
                const LogEvent::ptr &event) override {
        log.append(event->getFilename());
    }
};

class LineFormatItem : public LogFormatter::FormatterItem {
public:
    explicit LineFormatItem(const std::string &str = "") {
    }

    void format(std::string &log, const std::shared_ptr<Logger> &logger, LogLever::Lever lever,
                const LogEvent::ptr &event) override {
        log.append(std::to_string(event->getLine()));
    }
};

class NewLineFormatItem : public LogFormatter::FormatterItem {
public:
    explicit NewLineFormatItem(const std::string &str = "") {
    }

    void format(std::string &log, const std::shared_ptr<Logger> &logger, LogLever::Lever lever,
                const LogEvent::ptr &event) override {
        log.append("\n");
    }
};

class StringFormaItem : public LogFormatter::FormatterItem {
public:
    explicit StringFormaItem(std::string_view str) : m_string_(str) {
    }

    void format(std::string &log, const std::shared_ptr<Logger> &logger, LogLever::Lever lever,
                const LogEvent::ptr &event) override {
        log.append(m_string_);
    }

private:
    const std::string m_string_;
};


/**
 * 简单的状态机判断，提取pattern中的常规字符和模式字符
 *
 * 解析的过程就是从头到尾遍历，根据状态标志决定当前字符是常规字符还是模式字符
 *
 * 一共有两种状态，即正在解析常规字符和正在解析模板转义字符
 *
 * 比较麻烦的是%%d，后面可以接一对大括号指定时间格式，比如%%d{%%Y-%%m-%%d %%H:%%M:%%S}，这个状态需要特殊处理
 *
 * 一旦状态出错就停止解析，并设置错误标志，未识别的pattern转义字符也算出错
 */


void LogFormatter::init() {

    // 按顺序存储解析到的pattern项
    // 每个pattern包括一个整数类型和一个字符串，类型为0表示该pattern是常规字符串，为1表示该pattern需要转义
    // 日期格式单独用下面的dataformat存储
    std::vector<std::pair<int, std::string>> patterns;

    // 临时存储常规字符串
    std::string tmp;

    //存储时间日期的格式
    std::string dateFormat;

    //是否解析出错
    bool error = false;

    //设置初始状态 默认为解析模板
    LogFormatter::Parse status = LogFormatter::Parse::NORMAL;

    size_t i = 0;
    while (i < m_pattern_.size()) {
        //逐个字符进行解析
        std::string c(1, m_pattern_[i]);
        if (c == "%") {
            switch (status) {
                case LogFormatter::Parse::NORMAL: {
                    if (!tmp.empty()) {
                        //0表示常规字符
                        patterns.emplace_back(0, tmp);
                    }
                    tmp.clear();
                    //因为解析常规字符的时候碰到了% 所以要解析模板字符了
                    status = LogFormatter::Parse::TEMPLATE;
                    break;
                }
                case LogFormatter::Parse::TEMPLATE: {
                    //1表示模板字符
                    //在解析模板字符的时候碰到了%说明%事项表示常规字符说明要解析常规字符了 要将其转为常规字符的状态
                    patterns.emplace_back(1, c);
                    status = LogFormatter::Parse::NORMAL;
                    break;
                }
            }
            ++i;
            continue;
        }
            //碰到的不是%
        else {
            switch (status) {
                case LogFormatter::Parse::NORMAL: {
                    //持续解析常规字符串 直到遇见% 把解析后的常规的字符串放入patterns中
                    tmp += c;
                    ++i;
                    break;
                }
                case LogFormatter::Parse::TEMPLATE: {
                    //若是解析模板字符
                    patterns.emplace_back(1, c);
                    //插入完后 将其切回常规字符
                    status = LogFormatter::Parse::NORMAL;
                    ++i;
                    //此处又要对时间有特殊的处理 将%d后大括号中的内容放入dataformat
                    if (c == "d") {
                        if (i < m_pattern_.size() && m_pattern_[i] == '{') {
                            ++i;
                            while (i < m_pattern_.size() && m_pattern_[i] != '}') {
                                dateFormat.push_back(m_pattern_[i]);
                                ++i;
                            }
                            //说明其中的大括号没有闭合
                            if (m_pattern_[i] != '}') {
                                std::cerr << "parse dateFormat error" << std::endl;
                                error = true;
                            }
                            //没有出错的此时i的位置是'}'的位置 所以要加一
                            ++i;
                        }
                    }
                    break;
                }
            }
            if (error) break;
            continue;
        }
    }

    //退出循环后 也要将常规字符插入其中
    //以下是测试
    // if(!tmp.empty()){
    //     std::cout<<"tmp is"<<std::endl;
    //     patterns.emplace_back(std::make_pair(0,tmp));
    //     tmp.clear();
    // }
    //    // for debug
    // std::cout << "patterns:" << std::endl;
    // for(auto &v : patterns) {
    //     std::cout << "type = " << v.first << ", value = " << v.second << std::endl;
    // }
    // std::cout << "dataformat = " << dateFormat << std::endl;

    //使用map建立字符和 Item的关系
    static std::map<std::string, std::function<FormatterItem::ptr(const std::string &str)> > s_formatters = {
#define xx(str, func) \
    {#str,[](const std::string &fmt){return FormatterItem::ptr(new func(fmt));}}

            xx(m, MessageFormatItem),
            xx(p, LeverFormatItem),
            xx(c, NameFormatItem),
            xx(r, ElapseFormatItem),
            xx(f, FilenameFormatItem),
            xx(t, ThreadIdFormatItem),
            xx(F, FiberIdFormatItem),
            xx(n, NewLineFormatItem),
            xx(l, LineFormatItem),
#undef xx
    };
    for (auto &v: patterns) {
        if (v.first == 0) {
            //是常规字符
            m_items_.emplace_back(FormatterItem::ptr(new StringFormaItem(v.second)));
        }
            // 是模板(转义字符)字符
            // 日期做特殊的处理
        else if (v.second == "d") {
            m_items_.emplace_back(FormatterItem::ptr(new DateTimeFormatItem(dateFormat)));
        }
            //以下是模板字符的处理
        else {
            auto it = s_formatters.find(v.second);
            if (it == s_formatters.end()) {
                std::cerr << "error happend is " << v.second << std::endl;
                error = true;
                break;
            } else {
                //map中存在该字符对应的Item
                m_items_.emplace_back(it->second(v.second));
            }
        }
    }
    //这里必须直接报错 因为format的格式必须正确 以保证后续日志的正常产出
    if (error)
        abort();
}