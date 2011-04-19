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
#ifndef __RUNTIME_BASE_EXTENDED_LOGGER_H__
#define __RUNTIME_BASE_EXTENDED_LOGGER_H__

#include <util/logger.h>
#include <util/exception.h>
#include <util/stack_trace.h>
#include <runtime/base/complex_types.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class ExtendedLogger : public Logger {
public:
  static bool EnabledByDefault;

  // These logging functions will also print stacktrace at end of each message.
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

  // Log additional injected stacktrace.
  static void Log(bool err, CArrRef stackTrace, bool escape = true,
                  bool escapeMore = false);
  static std::string StringOfStackTrace(CArrRef stackTrace);

protected:
  virtual void log(bool err, const char *type, const Exception &e,
                   const char *file = NULL, int line = 0);
  virtual void log(bool err, const std::string &msg,
                   const StackTrace *stackTrace,
                   bool escape = true, bool escapeMore = false);

private:
  static void PrintStackTrace(FILE *f, CArrRef stackTrace,
                              bool escape = false, bool escapeMore = false);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __RUNTIME_BASE_EXTENDED_LOGGER_H__
