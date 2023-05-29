

#include "../include/LogLever.h"

using namespace wlog;

const char *LogLever::Tostring(LogLever::Lever lever_) {
    //#标识将数据加上一对引号 变成字符串
    //将日志等级输出
    switch (lever_) {
#define XX(name) \
    case LogLever::name:\
        return #name;
        XX(DEBUG)
        XX(INFO)
        XX(WARN)
        XX(ERROR)
        XX(FATAL)
        XX(NOTICE)
        XX(CRIT)
        XX(ALERT)
        //取消已经定义的宏
#undef XX
        default:
            return "UNKNOW";
    }
}