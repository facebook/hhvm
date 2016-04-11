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

struct StackTrace;
struct Exception;

struct ErrorLogFileData {
  ErrorLogFileData() {}
  ErrorLogFileData(const std::string& file, const std::string& symlink)
    : logFile(file)
    , symLink(symlink)
  {}
  std::string logFile;
  std::string symLink;
  bool isPipeOutput() const { return !logFile.empty() && logFile[0] == '|'; }
  bool hasTemplate() const { return logFile.find('%') != std::string::npos; }
};

struct Logger {
  enum LogLevelType {
    LogNone,
    LogError,
    LogWarning,
    LogInfo,
    LogVerbose
  };

  Logger()
    : m_isPipeOutput(false)
    , m_output(nullptr)
    , m_standardOut(stderr)
  { ResetPid(); }

  static bool AlwaysEscapeLog;
  static bool UseSyslog;
  static bool UseLogFile;
  static bool UseRequestLog;
  static bool UseCronolog;
  static LogLevelType LogLevel;
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
  static void UnlimitThreadMessages();

  typedef void (*PFUNC_LOG)(const char *header, const char *msg,
                            const char *ending, void *data);
  static void SetThreadHook(PFUNC_LOG func, void *data);

  static constexpr const char *DEFAULT = "Default";
  static void SetTheLogger(const std::string &name, Logger* newLogger);

  static char *EscapeString(const std::string &msg);

  static void SetStandardOut(const std::string &name, FILE* file);
  static Cronolog *CronoOutput(const std::string &name);
  static void SetOutput(const std::string &name, FILE *output, bool isPipe);
  static std::pair<FILE*, bool> GetOutput(const std::string &name);

  virtual ~Logger() { }
  static void ResetPid();

  static void FlushAll();

  virtual FILE* fileForStackTrace() { return output(); }

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
  virtual void log(LogLevelType level, const std::string &msg,
                   const StackTrace *stackTrace,
                   bool escape = false, bool escapeMore = false);
  // mainly intended for subclass of loggers that batch
  virtual void flush() {}

  // deduce where to write log
  virtual FILE* output();

  /**
   * What needs to be print for each line of logging. Currently it's
   * [machine:thread:datetime].
   */
  static std::string GetHeader();
  static pid_t s_pid;
protected:
  bool m_isPipeOutput;
  FILE *m_output;
  FILE* m_standardOut;
  Cronolog m_cronOutput;
  LogFileFlusher m_flusher;
  static std::map<std::string, Logger*> s_loggers;
};

///////////////////////////////////////////////////////////////////////////////
}

#include "hphp/util/logger-inl.h"

#endif // incl_HPHP_LOGGER_H_
