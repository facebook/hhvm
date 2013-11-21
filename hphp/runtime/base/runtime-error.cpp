/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/util/logger.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/*
 * Careful in these functions: they can be called when tl_regState is
 * REGSTATE_DIRTY.  VMExecutionContext::handleError is dirty-reg safe,
 * but evaluate other functions that you might need here.
 */

void raise_error(const std::string &msg) {
  int errnum = static_cast<int>(ErrorConstants::ErrorModes::ERROR);
  g_context->handleError(msg, errnum, false,
                         RuntimeOption::CallUserHandlerOnFatals ?
                         ExecutionContext::ErrorThrowMode::IfUnhandled :
                         ExecutionContext::ErrorThrowMode::Always,
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

void raise_error_without_first_frame(const std::string &msg) {
  int errnum = static_cast<int>(ErrorConstants::ErrorModes::ERROR);
  g_context->handleError(msg, errnum, false,
                         RuntimeOption::CallUserHandlerOnFatals ?
                         ExecutionContext::ErrorThrowMode::IfUnhandled :
                         ExecutionContext::ErrorThrowMode::Always,
                         "HipHop Fatal error: ",
                         true);
}

void raise_recoverable_error(const std::string &msg) {
  int errnum = static_cast<int>(ErrorConstants::ErrorModes::RECOVERABLE_ERROR);
  g_context->handleError(msg, errnum, true,
                         ExecutionContext::ErrorThrowMode::IfUnhandled,
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

static int64_t g_notice_counter = 0;

void raise_strict_warning(const std::string &msg) {
  if (RuntimeOption::NoticeFrequency <= 0 ||
      (g_notice_counter++) % RuntimeOption::NoticeFrequency != 0) {
    return;
  }
  int errnum = static_cast<int>(ErrorConstants::ErrorModes::STRICT);
  if (!g_context->errorNeedsHandling(errnum, true,
                                     ExecutionContext::ErrorThrowMode::Never)) {
    return;
  }
  g_context->handleError(msg, errnum, true,
                         ExecutionContext::ErrorThrowMode::Never,
                         "HipHop Strict Warning: ");
}

void raise_strict_warning(const char *fmt, ...) {
  if (RuntimeOption::NoticeFrequency <= 0 ||
      (g_notice_counter++) % RuntimeOption::NoticeFrequency != 0) {
    return;
  }
  std::string msg;
  int errnum = static_cast<int>(ErrorConstants::ErrorModes::STRICT);
  if (!g_context->errorNeedsHandling(errnum, true,
                                     ExecutionContext::ErrorThrowMode::Never)) {
    return;
  }
  va_list ap;
  va_start(ap, fmt);
  Util::string_vsnprintf(msg, fmt, ap);
  va_end(ap);
  g_context->handleError(msg, errnum, true,
                         ExecutionContext::ErrorThrowMode::Never,
                         "HipHop Strict Warning: ");
}

static int64_t g_warning_counter = 0;

void raise_warning(const std::string &msg) {
  if (RuntimeOption::WarningFrequency <= 0 ||
      (g_warning_counter++) % RuntimeOption::WarningFrequency != 0) {
    return;
  }
  int errnum = static_cast<int>(ErrorConstants::ErrorModes::WARNING);
  if (!g_context->errorNeedsHandling(errnum, true,
                                     ExecutionContext::ErrorThrowMode::Never)) {
    return;
  }
  g_context->handleError(msg, errnum, true,
                         ExecutionContext::ErrorThrowMode::Never,
                         "HipHop Warning: ");
}

void raise_warning(const char *fmt, ...) {
  if (RuntimeOption::WarningFrequency <= 0 ||
      (g_warning_counter++) % RuntimeOption::WarningFrequency != 0) {
    return;
  }
  std::string msg;
  int errnum = static_cast<int>(ErrorConstants::ErrorModes::WARNING);
  if (!g_context->errorNeedsHandling(errnum, true,
                                     ExecutionContext::ErrorThrowMode::Never)) {
    return;
  }
  va_list ap;
  va_start(ap, fmt);
  Util::string_vsnprintf(msg, fmt, ap);
  va_end(ap);
  g_context->handleError(msg, errnum, true,
                         ExecutionContext::ErrorThrowMode::Never,
                         "HipHop Warning: ");
}

/**
 * Warnings are currently sampled. raise_debugging can help when
 * migrating warnings to errors.
 *
 * In general, RaiseDebuggingFrequency should be kept at 1.
 */
static int64_t g_raise_debugging_counter = 0;

void raise_debugging(const std::string &msg) {
  if (RuntimeOption::RaiseDebuggingFrequency <= 0 ||
      (g_raise_debugging_counter++) %
      RuntimeOption::RaiseDebuggingFrequency != 0) {
    return;
  }

  g_context->handleError(msg,
                         static_cast<int>(ErrorConstants::ErrorModes::WARNING),
                         true,
                         ExecutionContext::ErrorThrowMode::Never,
                         "HipHop Warning: ");
}

void raise_debugging(const char *fmt, ...) {
  std::string msg;
  va_list ap;
  va_start(ap, fmt);
  Util::string_vsnprintf(msg, fmt, ap);
  va_end(ap);
  raise_debugging(msg);
}

void raise_notice(const std::string &msg) {
  if (RuntimeOption::NoticeFrequency <= 0 ||
      (g_notice_counter++) % RuntimeOption::NoticeFrequency != 0) {
    return;
  }
  int errnum = static_cast<int>(ErrorConstants::ErrorModes::NOTICE);
  if (!g_context->errorNeedsHandling(errnum, true,
                                     ExecutionContext::ErrorThrowMode::Never)) {
    return;
  }
  g_context->handleError(msg, errnum, true,
                         ExecutionContext::ErrorThrowMode::Never,
                         "HipHop Notice: ");
}

void raise_notice(const char *fmt, ...) {
  if (RuntimeOption::NoticeFrequency <= 0 ||
      (g_notice_counter++) % RuntimeOption::NoticeFrequency != 0) {
    return;
  }
  std::string msg;
  int errnum = static_cast<int>(ErrorConstants::ErrorModes::NOTICE);
  if (!g_context->errorNeedsHandling(errnum, true,
                                     ExecutionContext::ErrorThrowMode::Never)) {
    return;
  }
  va_list ap;
  va_start(ap, fmt);
  Util::string_vsnprintf(msg, fmt, ap);
  va_end(ap);
  g_context->handleError(msg, errnum, true,
                         ExecutionContext::ErrorThrowMode::Never,
                         "HipHop Notice: ");
}

void raise_param_type_warning(
    const char* func_name,
    int param_num,
    DataType expected_type,
    DataType actual_type) {
  // slice off fg1_
  if (strncmp(func_name, "fg1_", 4) == 0) {
    func_name += 4;
  } else if (strncmp(func_name, "tg1_", 4) == 0) {
    func_name += 4;
  }
  assert(param_num > 0);
  String expected_type_str = getDataTypeString(expected_type);
  String actual_type_str = getDataTypeString(actual_type);
  raise_warning(
    "%s() expects parameter %d to be %s, %s given",
    func_name,
    param_num,
    expected_type_str.c_str(),
    actual_type_str.c_str());
}

void raise_message(ErrorConstants::ErrorModes mode,
                   const char *fmt,
                   va_list ap) {
  std::string msg;
  Util::string_vsnprintf(msg, fmt, ap);
  raise_message(mode, msg);
}

void raise_message(ErrorConstants::ErrorModes mode, std::string &msg) {
  switch (mode) {
    case ErrorConstants::ErrorModes::ERROR:
      raise_error(msg);
      break;
    case ErrorConstants::ErrorModes::WARNING:
      raise_warning(msg);
      break;
    case ErrorConstants::ErrorModes::NOTICE:
    case ErrorConstants::ErrorModes::PHP_DEPRECATED:
      raise_notice(msg);
      break;
    default:
      always_assert(!"Unhandled type of error");
  }
}

///////////////////////////////////////////////////////////////////////////////
}

