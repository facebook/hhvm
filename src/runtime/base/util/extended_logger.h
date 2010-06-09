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
#ifndef __RUNTIME_BASE_EXTENDED_LOGGER_H__
#define __RUNTIME_BASE_EXTENDED_LOGGER_H__

#include <util/logger.h>
#include <util/exception.h>
#include <util/stack_trace.h>
#include <runtime/base/complex_types.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class ExtendedLogger : public Logger {
protected:
  virtual void log(const char *type, const Exception &e,
                   const char *file = NULL, int line = 0);
  virtual void log(const std::string &msg, const StackTrace *stackTrace,
                   bool escape = true);
private:
  // Log additional injected stacktrace.
  void Log(CArrRef stackTrace);
  void PrintStackTrace(FILE *f, CArrRef stackTrace);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __RUNTIME_BASE_EXTENDED_LOGGER_H__
