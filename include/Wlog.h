/**
 * @file    Wlog.h
 * @brief   宏模块，提供给用户使用的宏。
 */

#pragma once

#ifdef __linux__

#include "Logger.h"

#include <unistd.h>
#include <sys/syscall.h>


using namespace wlog;

#elif _WIN32

#include<windows.h>

#endif


#ifdef __linux__
#define WLOG_LOG_LEVER(logger, lever)\
    if(logger->getLevel() >=lever) \
        LoggerPackage(LogEvent::ptr(new LogEvent(logger,lever,__FILE__,__LINE__,0,0,time(0)))).getEvent()

#elif _WIN32
#define WLOG_LOG_LEVER(logger,lever)\
    if(logger->getLevel()<=lever)\
        LoggerPackage(LogEvent::ptr(new LogEvent(logger,lever,__FILE__,__LINE__,0,0,time(0)))).getEvent()
#endif

#define WLOG_GET_LOGMGR LoggerManger::getInstance()

#define WLOG_GET_LOGGER WLOG_GET_LOGMGR->getLogger()

#define WLOG_LOG_INFO   WLOG_LOG_LEVER(WLOG_GET_LOGGER,LogLever::Lever::INFO)

#define WLOG_LOG_DEBUG  WLOG_LOG_LEVER(WLOG_GET_LOGGER,LogLever::Lever::DEBUG)

#define WLOG_LOG_ERROR  WLOG_LOG_LEVER(WLOG_GET_LOGGER,LogLever::Lever::ERROR)

#define WLOG_LOG_FATAL  WLOG_LOG_LEVER(WLOG_GET_LOGGER,LogLever::Lever::FATAL)

#define WLOG_LOG_WARN   WLOG_LOG_LEVER(WLOG_GET_LOGGER,LogLever::Lever::WARN)

#define WLOG_LOG_ALERT  WLOG_LOG_LEVER(WLOG_GET_LOGGER,LogLever::Lever::ALERT)

#define WLOG_LOG_NOTICE WLOG_LOG_LEVER(WLOG_GET_LOGGER,LogLever::Lever::NOTICE)
