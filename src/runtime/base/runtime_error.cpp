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

#include <runtime/base/execution_context.h>
#include <runtime/base/runtime_option.h>
#include <util/logger.h>
#include "runtime_error.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

void raise_error_ex(const std::string &msg,
                    int64 errnum,
                    bool callUserHandler,
                    ThrowMode mode,
                    const std::string &prefix) {
  if (g_context->isInsideRaiseError()) return;
  try {
    g_context->setInsideRaiseError(true);

    ExtendedException ee(msg);
    bool handled = false;
    g_context->recordLastError(ee);
    if (callUserHandler) {
      handled = g_context->callUserErrorHandler(ee, errnum, false);
    }
    if (mode == AlwaysThrow || (mode == ThrowIfUnhandled && !handled)) {
      g_context->setInsideRaiseError(false);
      throw FatalErrorException(msg.c_str());
    }
    if (!handled &&
        (RuntimeOption::NoSilencer ||
         (g_context->getErrorReportingLevel() & errnum) != 0)) {
      Logger::Log(prefix.c_str(), ee);
    }
  } catch (...) {
    g_context->setInsideRaiseError(false);
    throw;
  }
  g_context->setInsideRaiseError(false);
}

void raise_error(const std::string &msg) {
  int64 errnum = 1LL;  // E_ERROR
  raise_error_ex(msg, errnum, false, AlwaysThrow, "HipHop Fatal error: ");
}

void raise_error(const char *fmt, ...) {
  std::string msg;
  va_list ap;
  va_start(ap, fmt);
  Logger::VSNPrintf(msg, fmt, ap);
  va_end(ap);
  raise_error(msg);
}

void raise_recoverable_error(const std::string &msg) {
  int64 errnum = 4096LL;  // E_RECOVERABLE_ERROR
  raise_error_ex(msg, errnum, true, ThrowIfUnhandled,
                 "HipHop Recoverable Fatal error: ");
}

void raise_recoverable_error(const char *fmt, ...) {
  std::string msg;
  va_list ap;
  va_start(ap, fmt);
  Logger::VSNPrintf(msg, fmt, ap);
  va_end(ap);
  raise_recoverable_error(msg);
}

static int64 g_warning_counter = 0;

void raise_warning(const std::string &msg) {
  if (RuntimeOption::WarningFrequency > 0 &&
      (++g_warning_counter) % RuntimeOption::WarningFrequency == 0) {
    int64 errnum = 2LL;  // E_WARNING
    raise_error_ex(msg, errnum, true, NeverThrow, "HipHop Warning:  ");
  }
}

void raise_warning(const char *fmt, ...) {
  std::string msg;
  va_list ap;
  va_start(ap, fmt);
  Logger::VSNPrintf(msg, fmt, ap);
  va_end(ap);
  raise_warning(msg);
}

static int64 g_notice_counter = 0;

void raise_notice(const std::string &msg) {
  if (RuntimeOption::NoticeFrequency > 0 &&
      (++g_notice_counter) % RuntimeOption::NoticeFrequency == 0) {
    int64 errnum = 8LL;  // E_NOTICE
    raise_error_ex(msg, errnum, true, NeverThrow, "HipHop Notice:  ");
  }
}

void raise_notice(const char *fmt, ...) {
  std::string msg;
  va_list ap;
  va_start(ap, fmt);
  Logger::VSNPrintf(msg, fmt, ap);
  va_end(ap);
  raise_notice(msg);
}

///////////////////////////////////////////////////////////////////////////////
}

