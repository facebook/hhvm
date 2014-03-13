/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_RUNTIME_BASE_EXTENDED_LOGGER_H_
#define incl_HPHP_RUNTIME_BASE_EXTENDED_LOGGER_H_

#include "hphp/util/logger.h"
#include "hphp/util/exception.h"
#include "hphp/util/stack-trace.h"
#include "hphp/runtime/base/complex-types.h"

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

  static void Error(const char *fmt, ...) ATTRIBUTE_PRINTF(1,2);
  static void Warning(const char *fmt, ...) ATTRIBUTE_PRINTF(1,2);
  static void Info(const char *fmt, ...) ATTRIBUTE_PRINTF(1,2);
  static void Verbose(const char *fmt, ...) ATTRIBUTE_PRINTF(1,2);

  // Log additional injected stacktrace.
  static void Log(LogLevelType level, const Array& stackTrace, bool escape = true,
                  bool escapeMore = false);

  // Convenience functions for stringifying.
  static std::string StringOfFrame(const Array& frame, int i, bool escape = false);
  static std::string StringOfStackTrace(const Array& stackTrace);

protected:
  virtual void log(LogLevelType level, const char *type, const Exception &e,
                   const char *file = nullptr, int line = 0);
  virtual void log(LogLevelType level, const std::string &msg,
                   const StackTrace *stackTrace,
                   bool escape = true, bool escapeMore = false);

private:
  static void PrintStackTrace(FILE *f, const Array& stackTrace,
                              bool escape = false, bool escapeMore = false);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_RUNTIME_BASE_EXTENDED_LOGGER_H_
