/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <string>
#include <stdarg.h>
#include "thread_local.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class StackTrace;
class Exception;
class Logger {
public:
  enum LogLevelType {
    LogNone,
    LogError,
    LogWarning,
    LogInfo,
    LogVerbose,
  };

  static bool UseLogAggregator;
  static bool UseLogFile;
  static FILE *Output;

  static LogLevelType LogLevel;

  static bool LogHeader;
  static bool LogNativeStackTrace;
  static std::string ExtraHeader;
  static int MaxMessagesPerRequest;

  static void Error(const std::string &msg);
  static void Warning(const std::string &msg);
  static void Info(const std::string &msg);
  static void Verbose(const std::string &msg);

  static void Error(const char *fmt, ...);
  static void Warning(const char *fmt, ...);
  static void Info(const char *fmt, ...);
  static void Verbose(const char *fmt, ...);

  // log messages without escaping
  static void RawError(const std::string &msg);
  static void RawWarning(const std::string &msg);
  static void RawInfo(const std::string &msg);
  static void RawVerbose(const std::string &msg);

  static void Log(const char *type, const Exception &e,
                  const char *file = NULL, int line = 0);

  static void Printf(std::string &msg, const char *fmt, ...);
  static void VSNPrintf(std::string &msg, const char *fmt, va_list ap);

  static void OnNewRequest();
  static void ResetRequestCount();

  static bool SetThreadLog(const char *file);
  static void ClearThreadLog();

  static void SetTheLogger(Logger* logger) {
    if (logger != NULL) {
      delete s_logger;
      s_logger = logger;
    }
  }

  virtual ~Logger() { }

protected:
  class ThreadData {
  public:
    ThreadData() : request(0), message(0), log(NULL) {}
    int request;
    int message;
    FILE *log;
  };
  static DECLARE_THREAD_LOCAL(ThreadData, s_threadData);

  static void Log(const char *fmt, va_list ap);
  static void Log(const std::string &msg, const StackTrace *stackTrace,
                  bool escape = true);

  /**
   * For subclasses to override, e.g., to support injected stack trace.
   */
  virtual void log(const char *type, const Exception &e,
                   const char *file = NULL, int line = 0);
  virtual void log(const std::string &msg, const StackTrace *stackTrace,
                   bool escape = true);

  /**
   * What needs to be print for each line of logging. Currently it's
   * [machine:thread:datetime].
   */
  static std::string GetHeader();

  static char *EscapeString(const std::string &msg);

private:
  static Logger *s_logger;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __LOGGER_H__
