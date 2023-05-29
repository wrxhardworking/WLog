#include "../include/LogEvent.h"

#include <sys/syscall.h>
#include <unistd.h>

using namespace wlog;

namespace {
    //这是一个线程局部变量每一个线程都有这个变量的副本
    thread_local uint32_t tid = 0;
}
class Tid {
public:
    static void setTid() {
        tid = ::syscall(SYS_gettid);
    }
};

LogEvent::LogEvent(const std::shared_ptr<Logger> & logger, LogLever::Lever lever, const char *filename, uint32_t line,
                   uint32_t elapse, uint32_t fiber, int32_t time)
        : m_logger_(logger), m_lever_(lever), m_filename_(filename), m_line_(line), m_elapse_(elapse),
          m_fiberId(fiber), m_time_(time), m_threadId(tid) {
    if (m_threadId == 0) {
        Tid::setTid();
        m_threadId = tid;
    }
}

