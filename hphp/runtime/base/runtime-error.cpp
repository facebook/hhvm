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
  raise_message(ErrorConstants::ErrorModes::ERROR, false, msg);
}

void raise_error(const char *fmt, ...) {
  std::string msg;
  va_list ap;
  va_start(ap, fmt);
  string_vsnprintf(msg, fmt, ap);
  va_end(ap);
  raise_error(msg);
}

/*
 * This does not call raise_message because the mode of the error differs
 * depending on the runtime option unlike the regular raise_error.
 */
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
  raise_message(ErrorConstants::ErrorModes::RECOVERABLE_ERROR, false, msg);
}

void raise_recoverable_error_without_first_frame(const std::string &msg) {
  raise_message(ErrorConstants::ErrorModes::RECOVERABLE_ERROR, true, msg);
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
  raise_message(ErrorConstants::ErrorModes::STRICT, false, msg);
}

void raise_strict_warning_without_first_frame(const std::string &msg) {
  raise_message(ErrorConstants::ErrorModes::STRICT, true, msg);
}

void raise_strict_warning(const char *fmt, ...) {
  std::string msg;
  va_list ap;
  va_start(ap, fmt);
  string_vsnprintf(msg, fmt, ap);
  va_end(ap);
  raise_strict_warning(msg);
}

static int64_t g_warning_counter = 0;

void raise_warning(const std::string &msg) {
  raise_message(ErrorConstants::ErrorModes::WARNING, false, msg);
}

void raise_warning_without_first_frame(const std::string &msg) {
  raise_message(ErrorConstants::ErrorModes::WARNING, true, msg);
}

void raise_warning(const char *fmt, ...) {
  std::string msg;
  va_list ap;
  va_start(ap, fmt);
  string_vsnprintf(msg, fmt, ap);
  va_end(ap);
  raise_warning(msg);
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
  raise_message(ErrorConstants::ErrorModes::WARNING, false, msg);
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
  raise_message(ErrorConstants::ErrorModes::NOTICE, false, msg);
}

void raise_notice_without_first_frame(const std::string &msg) {
  raise_message(ErrorConstants::ErrorModes::NOTICE, true, msg);
}

void raise_notice(const char *fmt, ...) {
  std::string msg;
  va_list ap;
  va_start(ap, fmt);
  string_vsnprintf(msg, fmt, ap);
  va_end(ap);
  raise_notice(msg);
}

void raise_deprecated(const std::string &msg) {
  raise_message(ErrorConstants::ErrorModes::PHP_DEPRECATED, false, msg);
}

void raise_deprecated_without_first_frame(const std::string &msg) {
  raise_message(ErrorConstants::ErrorModes::PHP_DEPRECATED, true, msg);
}

void raise_deprecated(const char *fmt, ...) {
  std::string msg;
  va_list ap;
  va_start(ap, fmt);
  string_vsnprintf(msg, fmt, ap);
  va_end(ap);
  raise_message(ErrorConstants::ErrorModes::PHP_DEPRECATED, false, msg);
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
  raise_message(mode, false, msg);
}

void raise_message(ErrorConstants::ErrorModes mode,
                   const char *fmt,
                   ...) {
  std::string msg;
  va_list ap;
  va_start(ap, fmt);
  string_vsnprintf(msg, fmt, ap);
  va_end(ap);
  raise_message(mode, false, msg);
}

#define HANDLE_ERROR(userHandle, mode, str, skip)                       \
  g_context->handleError(msg, errnum, userHandle,                       \
                         ExecutionContext::ErrorThrowMode::mode,        \
                         str,                                           \
                         skip);

void raise_message(ErrorConstants::ErrorModes mode,
                   bool skipTop,
                   const std::string &msg) {
  int errnum = static_cast<int>(mode);
  if (mode == ErrorConstants::ErrorModes::ERROR) {
    HANDLE_ERROR(false, Always, "\nFatal error: ", skipTop);
  } else if (mode == ErrorConstants::ErrorModes::RECOVERABLE_ERROR) {
    HANDLE_ERROR(true, IfUnhandled, "\nCatchable Fatal error: ", skipTop);
  } else if (!g_context->errorNeedsHandling(errnum, true,
                              ExecutionContext::ErrorThrowMode::Never)) {
    return;
  } else if (mode == ErrorConstants::ErrorModes::WARNING) {
    if (RuntimeOption::WarningFrequency <= 0 ||
        (g_warning_counter++) % RuntimeOption::WarningFrequency != 0) {
      return;
    }
    HANDLE_ERROR(true, Never, "\nWarning: ", skipTop);
  } else if (RuntimeOption::NoticeFrequency <= 0 ||
             (g_notice_counter++) % RuntimeOption::NoticeFrequency != 0) {
    return;
  } else {
    switch (mode) {
      case ErrorConstants::ErrorModes::STRICT:
        HANDLE_ERROR(true, Never, "\nStrict Warning: ", skipTop);
        break;
      case ErrorConstants::ErrorModes::NOTICE:
        HANDLE_ERROR(true, Never, "\nNotice: ", skipTop);
        break;
      case ErrorConstants::ErrorModes::PHP_DEPRECATED:
        HANDLE_ERROR(true, Never, "\nDeprecated: ", skipTop);
        break;
      default:
        always_assert(!"Unhandled type of error");
    }
  }
}
///////////////////////////////////////////////////////////////////////////////
}
