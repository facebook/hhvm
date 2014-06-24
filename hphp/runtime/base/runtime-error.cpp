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

#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/repo-global-data.h"
#include "hphp/util/logger.h"
#include "hphp/util/string-vsnprintf.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/*
 * Careful in these functions: they can be called when tl_regState is
 * REGSTATE_DIRTY.  ExecutionContext::handleError is dirty-reg safe,
 * but evaluate other functions that you might need here.
 */

void raise_error(const std::string &msg) {
  auto const errnum = static_cast<int>(ErrorConstants::ErrorModes::ERROR);
  g_context->handleError(msg, errnum, false,
                         ExecutionContext::ErrorThrowMode::Always,
                         "\nFatal error: ");
}

void raise_error(const char *fmt, ...) {
  std::string msg;
  va_list ap;
  va_start(ap, fmt);
  string_vsnprintf(msg, fmt, ap);
  va_end(ap);
  raise_error(msg);
}

void raise_error_without_first_frame(const std::string &msg) {
  int errnum = static_cast<int>(ErrorConstants::ErrorModes::ERROR);
  g_context->handleError(msg, errnum, false,
                         RuntimeOption::CallUserHandlerOnFatals ?
                         ExecutionContext::ErrorThrowMode::IfUnhandled :
                         ExecutionContext::ErrorThrowMode::Always,
                         "\nFatal error: ",
                         true);
}

void raise_recoverable_error(const std::string &msg) {
  int errnum = static_cast<int>(ErrorConstants::ErrorModes::RECOVERABLE_ERROR);
  g_context->handleError(msg, errnum, true,
                         ExecutionContext::ErrorThrowMode::IfUnhandled,
                         "\nCatchable Fatal error: ");
}

void raise_typehint_error(const std::string& msg) {
  raise_recoverable_error(msg);
  if (RuntimeOption::RepoAuthoritative && Repo::global().HardTypeHints) {
    raise_error("Error handler tried to recover from typehint violation");
  }
}

void raise_disallowed_dynamic_call(const std::string& msg) {
  if (RuntimeOption::RepoAuthoritative &&
      Repo::global().DisallowDynamicVarEnvFuncs) {
    raise_error(msg);
  }
  raise_hack_strict(RuntimeOption::DisallowDynamicVarEnvFuncs,
                    "disallow_dynamic_var_env_funcs",
                    msg);
}

void raise_recoverable_error(const char *fmt, ...) {
  std::string msg;
  va_list ap;
  va_start(ap, fmt);
  string_vsnprintf(msg, fmt, ap);
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
                         "\nStrict Warning: ");
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
  string_vsnprintf(msg, fmt, ap);
  va_end(ap);
  g_context->handleError(msg, errnum, true,
                         ExecutionContext::ErrorThrowMode::Never,
                         "\nStrict Warning: ");
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
                         "\nWarning: ");
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
  string_vsnprintf(msg, fmt, ap);
  va_end(ap);
  g_context->handleError(msg, errnum, true,
                         ExecutionContext::ErrorThrowMode::Never,
                         "\nWarning: ");
}

/**
 * For use with the HackStrictOption settings. This will warn, error, or do
 * nothing depending on what the user chose for the option. The second param
 * should be the ini setting name after "hhvm.hack."
 */
void raise_hack_strict(HackStrictOption option, const char *ini_setting,
                       const std::string& msg) {
  if (option == HackStrictOption::WARN) {
    raise_warning(std::string("(hhvm.hack.") + ini_setting + "=warn) " + msg);
  } else if (option == HackStrictOption::ERROR) {
    raise_error(std::string("(hhvm.hack.") + ini_setting + "=error) " + msg);
  }
}

void raise_hack_strict(HackStrictOption option, const char *ini_setting,
                       const char *fmt, ...) {
  std::string msg;
  va_list ap;
  va_start(ap, fmt);
  string_vsnprintf(msg, fmt, ap);
  va_end(ap);
  raise_hack_strict(option, ini_setting, msg);
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
                         "\nWarning: ");
}

void raise_debugging(const char *fmt, ...) {
  std::string msg;
  va_list ap;
  va_start(ap, fmt);
  string_vsnprintf(msg, fmt, ap);
  va_end(ap);
  raise_debugging(msg);
}

void raise_notice(const std::string &msg) {
  raise_message(ErrorConstants::ErrorModes::NOTICE, msg);
}

void raise_notice(const char *fmt, ...) {
  std::string msg;
  va_list ap;
  va_start(ap, fmt);
  string_vsnprintf(msg, fmt, ap);
  va_end(ap);
  raise_message(ErrorConstants::ErrorModes::NOTICE, msg);
}

void raise_deprecated(const std::string &msg) {
  raise_message(ErrorConstants::ErrorModes::PHP_DEPRECATED, msg);
}

void raise_deprecated(const char *fmt, ...) {
  std::string msg;
  va_list ap;
  va_start(ap, fmt);
  string_vsnprintf(msg, fmt, ap);
  va_end(ap);
  raise_message(ErrorConstants::ErrorModes::PHP_DEPRECATED, msg);
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
  string_vsnprintf(msg, fmt, ap);
  raise_message(mode, msg);
}

void raise_message(ErrorConstants::ErrorModes mode,
                   const char *fmt,
                   ...) {
  std::string msg;
  va_list ap;
  va_start(ap, fmt);
  string_vsnprintf(msg, fmt, ap);
  va_end(ap);
  raise_message(mode, msg);
}

void raise_message(ErrorConstants::ErrorModes mode, const std::string &msg) {
  switch (mode) {
    case ErrorConstants::ErrorModes::ERROR:
      raise_error(msg);
      break;
    case ErrorConstants::ErrorModes::WARNING:
      raise_warning(msg);
      break;
    case ErrorConstants::ErrorModes::NOTICE:
    case ErrorConstants::ErrorModes::PHP_DEPRECATED: {
        // This is here rather than in the individual functions to reduce the
        // copy+paste
        if (RuntimeOption::NoticeFrequency <= 0 ||
            (g_notice_counter++) % RuntimeOption::NoticeFrequency != 0) {
          break;
        }
        int errnum = static_cast<int>(mode);
        if (!g_context->errorNeedsHandling(errnum, true,
                                      ExecutionContext::ErrorThrowMode::Never)) {
          return;
        }
        if (mode == ErrorConstants::ErrorModes::NOTICE) {
          g_context->handleError(msg, errnum, true,
                                 ExecutionContext::ErrorThrowMode::Never,
                                 "\nNotice: ");
        } else {
          g_context->handleError(msg, errnum, true,
                                 ExecutionContext::ErrorThrowMode::Never,
                                 "\nDeprecated: ");
        }
      }
      break;
    default:
      always_assert(!"Unhandled type of error");
  }
}

///////////////////////////////////////////////////////////////////////////////
}
