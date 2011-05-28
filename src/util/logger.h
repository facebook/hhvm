/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
#include <util/thread_local.h>
#include <util/cronolog.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class StackTrace;
class Exception;

class LogFileData {
public:
  LogFileData() : log(NULL), bytesWritten(0), prevBytesWritten(0) {}
  LogFileData(FILE *f) : log(f), bytesWritten(0), prevBytesWritten(0) {}
  FILE *log;
  int bytesWritten;
  int prevBytesWritten;
};

class Logger {
public:
  enum LogLevelType {
    LogNone,
    LogError,
    LogWarning,
    LogInfo,
    LogVerbose
  };

  static bool UseLogAggregator;
  static bool UseLogFile;
  static bool UseCronolog;
  static int DropCacheChunkSize;
  static FILE *Output;
  static Cronolog cronOutput;
  static LogLevelType LogLevel;
  static int bytesWritten;
  static int prevBytesWritten;
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

  static void Log(bool err, const char *type, const Exception &e,
                  const char *file = NULL, int line = 0);

  static void OnNewRequest();
  static void ResetRequestCount();

  static bool SetThreadLog(const char *file);
  static void ClearThreadLog();
  static void SetNewOutput(FILE *output);

  typedef void (*PFUNC_LOG)(const char *header, const char *msg,
                            const char *ending, void *data);
  static void SetThreadHook(PFUNC_LOG func, void *data);

  static void SetTheLogger(Logger* logger) {
    if (logger != NULL) {
      delete s_logger;
      s_logger = logger;
    }
  }

  static bool checkDropCache(int &bytesWritten, int &prevBytesWritten,
                             FILE *f);
  static char *EscapeString(const std::string &msg);

  virtual ~Logger() { }

protected:
  class ThreadData {
  public:
    ThreadData() : request(0), message(0), log(NULL), hook(NULL) {}
    int request;
    int message;
    int bytesWritten;
    int prevBytesWritten;
    FILE *log;
    PFUNC_LOG hook;
    void *hookData;
  };
  static DECLARE_THREAD_LOCAL(ThreadData, s_threadData);

  static void Log(bool err, const char *fmt, va_list ap);
  static void LogEscapeMore(bool err, const char *fmt, va_list ap);
  static void Log(bool err, const std::string &msg,
                  const StackTrace *stackTrace,
                  bool escape = true, bool escapeMore = false);

  /**
   * For subclasses to override, e.g., to support injected stack trace.
   */
  virtual void log(bool err, const char *type, const Exception &e,
                   const char *file = NULL, int line = 0);
  virtual void log(bool err, const std::string &msg,
                   const StackTrace *stackTrace,
                   bool escape = true, bool escapeMore = false);

  /**
   * What needs to be print for each line of logging. Currently it's
   * [machine:thread:datetime].
   */
  static std::string GetHeader();
private:
  static Logger *s_logger;

};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __LOGGER_H__
