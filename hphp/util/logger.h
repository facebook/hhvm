/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include <chrono>
#include <cstdarg>
#include <string>

#include "hphp/util/cronolog.h"
#include "hphp/util/log-file-flusher.h"
#include "hphp/util/service-data.h"
#include "hphp/util/thread-local.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct StackTrace;
struct Exception;

struct ErrorLogFileData {
  ErrorLogFileData() {}
  ErrorLogFileData(const std::string& file, const std::string& symlink, int mpl)
    : logFile(file)
    , symLink(symlink)
    , periodMultiplier(mpl)
  {}
  std::string logFile;
  std::string symLink;
  int periodMultiplier;
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
  static void SetBatchSize(size_t bsize);
  static void SetFlushTimeout(std::chrono::milliseconds timeoutMs);

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
    TYPE_SCAN_CONSERVATIVE_FIELD(hookData);
  };
  static DECLARE_THREAD_LOCAL(ThreadData, s_threadData);

  static void LogImpl(LogLevelType level, const std::string &msg,
                      const StackTrace *stackTrace,
                      bool escape = false, bool escapeMore = false);

  static inline bool IsEnabled() {
    return Logger::UseLogFile || Logger::UseSyslog;
  }

  static int GetSyslogLevel(LogLevelType level);

  friend struct ExtendedLogger;

  // For subclasses to override, e.g., to support injected stack trace.
  // Returns (lines, bytes) it's going to output. Used for monitoring growth.
  virtual std::pair<int, int> log(LogLevelType level, const std::string& msg,
                                  const StackTrace* stackTrace,
                                  bool escape = false, bool escapeMore = false);
  // Intended for subclass of loggers that batch.
  // Returns (lines, bytes) it's going to output. Used for monitoring growth.
  virtual std::pair<int, int> flush() { return std::make_pair(0, 0); }

  virtual void setBatchSize(size_t bsize) {}

  // flush the log after this timeout (in milliseconds) has been exceeded.
  // 0 will disable the timeout and flush only when the batch size has been
  // met.
  virtual void setFlushTimeout(std::chrono::milliseconds timeoutMs) {}

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
  // bytes and lines the DEFAULT logger has written
  static ServiceData::ExportedCounter* s_errorLines;
  static ServiceData::ExportedCounter* s_errorBytes;
};

///////////////////////////////////////////////////////////////////////////////
}

#include "hphp/util/logger-inl.h"

#endif // incl_HPHP_LOGGER_H_
