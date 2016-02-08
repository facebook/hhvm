/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_LOGGER_H_
#define incl_HPHP_LOGGER_H_

#include <atomic>
#include <string>
#include <stdarg.h>

#include "hphp/util/thread-local.h"
#include "hphp/util/cronolog.h"
#include "hphp/util/log-file-flusher.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class StackTrace;
class Exception;

struct Logger {
  enum LogLevelType {
    LogNone,
    LogError,
    LogWarning,
    LogInfo,
    LogVerbose
  };

  Logger(): m_standardOut(stderr) { ResetPid(); }

  static bool AlwaysEscapeLog;
  static bool UseSyslog;
  static bool UseLogFile;
  static bool UseRequestLog;
  static bool IsPipeOutput;
  static bool UseCronolog;
  static FILE *Output;
  static Cronolog cronOutput;
  static LogLevelType LogLevel;
  static LogFileFlusher flusher;
  static bool LogHeader;
  static bool LogNativeStackTrace;
  static std::string ExtraHeader;
  static int MaxMessagesPerRequest;
  static bool Escape;

  static void Error(const std::string &msg);
  static void Warning(const std::string &msg);
  static void Info(const std::string &msg);
  static void Verbose(const std::string &msg);

  static void Error(ATTRIBUTE_PRINTF_STRING const char *fmt, ...)
    ATTRIBUTE_PRINTF(1,2);
  static void Warning(ATTRIBUTE_PRINTF_STRING const char *fmt, ...)
    ATTRIBUTE_PRINTF(1,2);
  static void Info(ATTRIBUTE_PRINTF_STRING const char *fmt, ...)
    ATTRIBUTE_PRINTF(1,2);
  static void Verbose(ATTRIBUTE_PRINTF_STRING const char *fmt, ...)
    ATTRIBUTE_PRINTF(1,2);

  template<typename... Args> static void FError(Args&&... args);
  template<typename... Args> static void FWarning(Args&&... args);
  template<typename... Args> static void FInfo(Args&&... args);
  template<typename... Args> static void FVerbose(Args&&... args);

  static void Log(LogLevelType level, const char *type, const Exception &e,
                  const char *file = nullptr, int line = 0);
  static void OnNewRequest();
  static void ResetRequestCount();

  static bool SetThreadLog(const char *file, bool threadOnly);
  static void ClearThreadLog();
  static void SetNewOutput(FILE *output);
  static void UnlimitThreadMessages();

  typedef void (*PFUNC_LOG)(const char *header, const char *msg,
                            const char *ending, void *data);
  static void SetThreadHook(PFUNC_LOG func, void *data);

  static void SetTheLogger(Logger* logger) {
    if (logger != nullptr) {
      delete s_logger;
      s_logger = logger;
    }
  }

  static char *EscapeString(const std::string &msg);

  static FILE *GetStandardOut(LogLevelType level);
  static void SetStandardOut(FILE*);

  virtual ~Logger() { }
  static void ResetPid();

protected:
  struct ThreadData {
    int request{0};
    int message{0};
    LogFileFlusher flusher;
    FILE *log{nullptr};
    bool threadLogOnly{false};
    PFUNC_LOG hook{nullptr};
    void *hookData;
  };
  static DECLARE_THREAD_LOCAL(ThreadData, s_threadData);

  static void Log(LogLevelType level,
    ATTRIBUTE_PRINTF_STRING const char *fmt, va_list ap) ATTRIBUTE_PRINTF(2,0);
  static void LogEscapeMore(LogLevelType level,
    ATTRIBUTE_PRINTF_STRING const char *fmt, va_list ap) ATTRIBUTE_PRINTF(2,0);
  static void Log(LogLevelType level, const std::string &msg,
                  const StackTrace *stackTrace,
                  bool escape = false, bool escapeMore = false);

  static inline bool IsEnabled() {
    return Logger::UseLogFile || Logger::UseSyslog;
  }

  static int GetSyslogLevel(LogLevelType level);

  /**
   * For subclasses to override, e.g., to support injected stack trace.
   */
  virtual void log(LogLevelType level, const char *type, const Exception &e,
                   const char *file = nullptr, int line = 0);
  virtual void log(LogLevelType level, const std::string &msg,
                   const StackTrace *stackTrace,
                   bool escape = false, bool escapeMore = false);

  /**
   * What needs to be print for each line of logging. Currently it's
   * [machine:thread:datetime].
   */
  static std::string GetHeader();
  static pid_t s_pid;
private:
  static Logger *s_logger;
  FILE* m_standardOut;
};

///////////////////////////////////////////////////////////////////////////////
}

#include "hphp/util/logger-inl.h"

#endif // incl_HPHP_LOGGER_H_
