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

#include <runtime/base/execution_context.h>
#include <runtime/base/runtime_option.h>
#include <util/logger.h>
#include "runtime_error.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

void raise_error(const std::string &msg) {
  int errnum = ErrorConstants::ERROR;
  g_context->handleError(msg, errnum, false,
                         RuntimeOption::CallUserHandlerOnFatals ?
                         ExecutionContext::ThrowIfUnhandled :
                         ExecutionContext::AlwaysThrow,
                         "HipHop Fatal error: ");
}

void raise_error(const char *fmt, ...) {
  std::string msg;
  va_list ap;
  va_start(ap, fmt);
  Util::string_vsnprintf(msg, fmt, ap);
  va_end(ap);
  raise_error(msg);
}

void raise_recoverable_error(const std::string &msg) {
  int errnum = ErrorConstants::RECOVERABLE_ERROR;
  g_context->handleError(msg, errnum, true,
                         ExecutionContext::ThrowIfUnhandled,
                         "HipHop Recoverable Fatal error: ");
}

void raise_recoverable_error(const char *fmt, ...) {
  std::string msg;
  va_list ap;
  va_start(ap, fmt);
  Util::string_vsnprintf(msg, fmt, ap);
  va_end(ap);
  raise_recoverable_error(msg);
}

void raise_strict_warning(const std::string &msg) {
  int errnum = ErrorConstants::STRICT;
  if (!g_context->errorNeedsHandling(errnum, true,
                                     ExecutionContext::NeverThrow)) {
    return;
  }
  g_context->handleError(msg, errnum, true,
                         ExecutionContext::NeverThrow,
                         "HipHop Strict Warning:  ");
}

void raise_strict_warning(const char *fmt, ...) {
  std::string msg;
  int errnum = ErrorConstants::STRICT;
  if (!g_context->errorNeedsHandling(errnum, true,
                                     ExecutionContext::NeverThrow)) {
    return;
  }
  va_list ap;
  va_start(ap, fmt);
  Util::string_vsnprintf(msg, fmt, ap);
  va_end(ap);
  g_context->handleError(msg, errnum, true,
                         ExecutionContext::NeverThrow,
                         "HipHop Strict Warning:  ");
}

static int64 g_warning_counter = 0;

void raise_warning(const std::string &msg) {
  if (RuntimeOption::WarningFrequency <= 0 ||
      (g_warning_counter++) % RuntimeOption::WarningFrequency != 0) {
    return;
  }
  int errnum = ErrorConstants::WARNING;
  if (!g_context->errorNeedsHandling(errnum, true,
                                     ExecutionContext::NeverThrow)) {
    return;
  }
  g_context->handleError(msg, errnum, true,
                         ExecutionContext::NeverThrow,
                         "HipHop Warning:  ");
}

void raise_warning(const char *fmt, ...) {
  std::string msg;
  if (RuntimeOption::WarningFrequency <= 0 ||
      (g_warning_counter++) % RuntimeOption::WarningFrequency != 0) {
    return;
  }
  int errnum = ErrorConstants::WARNING;
  if (!g_context->errorNeedsHandling(errnum, true,
                                     ExecutionContext::NeverThrow)) {
    return;
  }
  va_list ap;
  va_start(ap, fmt);
  Util::string_vsnprintf(msg, fmt, ap);
  va_end(ap);
  g_context->handleError(msg, errnum, true,
                         ExecutionContext::NeverThrow,
                         "HipHop Warning:  ");
}

static int64 g_notice_counter = 0;

void raise_notice(const std::string &msg) {
  if (RuntimeOption::NoticeFrequency <= 0 ||
      (g_notice_counter++) % RuntimeOption::NoticeFrequency != 0) {
    return;
  }
  int errnum = ErrorConstants::NOTICE;
  if (!g_context->errorNeedsHandling(errnum, true,
                                     ExecutionContext::NeverThrow)) {
    return;
  }
  g_context->handleError(msg, errnum, true,
                         ExecutionContext::NeverThrow,
                         "HipHop Notice:  ");
}

void raise_notice(const char *fmt, ...) {
  std::string msg;
  if (RuntimeOption::NoticeFrequency <= 0 ||
      (g_notice_counter++) % RuntimeOption::NoticeFrequency != 0) {
    return;
  }
  int errnum = ErrorConstants::NOTICE;
  if (!g_context->errorNeedsHandling(errnum, true,
                                     ExecutionContext::NeverThrow)) {
    return;
  }
  va_list ap;
  va_start(ap, fmt);
  Util::string_vsnprintf(msg, fmt, ap);
  va_end(ap);
  g_context->handleError(msg, errnum, true,
                         ExecutionContext::NeverThrow,
                         "HipHop Notice:  ");
}

///////////////////////////////////////////////////////////////////////////////
}

