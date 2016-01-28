/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/repo-global-data.h"
#include "hphp/runtime/vm/vm-regs.h"
#include "hphp/util/logger.h"
#include "hphp/util/string-vsnprintf.h"

#ifdef ERROR
# undef ERROR
#endif
#ifdef STRICT
# undef STRICT
#endif

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/*
 * Careful in these functions: they can be called when tl_regState is
 * REGSTATE_DIRTY.  ExecutionContext::handleError is dirty-reg safe,
 * but evaluate other functions that you might need here.
 */

void raise_error(const std::string &msg) {
  g_context->handleError(msg, static_cast<int>(ErrorMode::ERROR), false,
                         ExecutionContext::ErrorThrowMode::Always,
                         "\nFatal error: ",
                         false);
  always_assert(0);
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
  g_context->handleError(msg, static_cast<int>(ErrorMode::ERROR), false,
                         ExecutionContext::ErrorThrowMode::Always,
                         "\nFatal error: ",
                         true);
  always_assert(0);
}

void raise_recoverable_error(const std::string &msg) {
  g_context->handleError(
    msg, static_cast<int>(ErrorMode::RECOVERABLE_ERROR), true,
    ExecutionContext::ErrorThrowMode::IfUnhandled,
    "\nCatchable Fatal error: ",
    false);
}

void raise_recoverable_error_without_first_frame(const std::string &msg) {
  g_context->handleError(
    msg, static_cast<int>(ErrorMode::RECOVERABLE_ERROR), true,
    ExecutionContext::ErrorThrowMode::IfUnhandled,
    "\nCatchable Fatal error: ",
    true);
}

void raise_typehint_error(const std::string& msg) {
  if (RuntimeOption::PHP7_EngineExceptions) {
    VMRegAnchor _;
    SystemLib::throwTypeErrorObject(msg);
  }
  raise_recoverable_error(msg);
  if (RuntimeOption::RepoAuthoritative && Repo::global().HardTypeHints) {
    raise_error("Error handler tried to recover from typehint violation");
  }
}

void raise_return_typehint_error(const std::string& msg) {
  if (RuntimeOption::PHP7_EngineExceptions) {
    VMRegAnchor _;
    SystemLib::throwTypeErrorObject(msg);
  }
  raise_recoverable_error(msg);
  if (RuntimeOption::EvalCheckReturnTypeHints >= 3 ||
      (RuntimeOption::RepoAuthoritative &&
       Repo::global().HardReturnTypeHints)) {
    raise_error("Error handler tried to recover from a return typehint "
                "violation");
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

static bool notice_freq_check(ErrorMode mode) {
  if (RuntimeOption::NoticeFrequency <= 0 ||
      g_notice_counter++ % RuntimeOption::NoticeFrequency != 0) {
    return false;
  }
  return g_context->errorNeedsHandling(
    static_cast<int>(mode), true, ExecutionContext::ErrorThrowMode::Never);
}

#define HANDLE_ERROR(userHandle, throwMode, str, skip)                  \
  g_context->handleError(msg, static_cast<int>(mode), userHandle,       \
                         ExecutionContext::ErrorThrowMode::throwMode,   \
                         str,                                           \
                         skip);

static void raise_notice_helper(ErrorMode mode, bool skipTop,
                                const std::string& msg) {
  switch (mode) {
    case ErrorMode::STRICT:
      HANDLE_ERROR(true, Never, "\nStrict Warning: ", skipTop);
      break;
    case ErrorMode::NOTICE:
      HANDLE_ERROR(true, Never, "\nNotice: ", skipTop);
      break;
    case ErrorMode::PHP_DEPRECATED:
      HANDLE_ERROR(true, Never, "\nDeprecated: ", skipTop);
      break;
    default:
      always_assert(!"Unhandled type of error");
  }
}

void raise_strict_warning(const std::string &msg) {
  if (notice_freq_check(ErrorMode::STRICT)) {
    raise_notice_helper(ErrorMode::STRICT, false, msg);
  }
}

void raise_strict_warning_without_first_frame(const std::string &msg) {
  if (notice_freq_check(ErrorMode::STRICT)) {
    raise_notice_helper(ErrorMode::STRICT, true, msg);
  }
}

void raise_strict_warning(const char *fmt, ...) {
  if (!notice_freq_check(ErrorMode::STRICT)) return;

  std::string msg;
  va_list ap;
  va_start(ap, fmt);
  string_vsnprintf(msg, fmt, ap);
  va_end(ap);
  raise_notice_helper(ErrorMode::STRICT, false, msg);
}

static int64_t g_warning_counter = 0;

bool warning_freq_check() {
  if (RuntimeOption::WarningFrequency <= 0 ||
      g_warning_counter++ % RuntimeOption::WarningFrequency != 0) {
    return false;
  }
  return g_context->errorNeedsHandling(
    static_cast<int>(ErrorMode::WARNING), true,
    ExecutionContext::ErrorThrowMode::Never);
}

void raise_warning_helper(bool skipTop, const std::string& msg) {
  auto mode = ErrorMode::WARNING;
  HANDLE_ERROR(true, Never, "\nWarning: ", skipTop);
}

void raise_warning(const std::string &msg) {
  if (warning_freq_check()) {
    raise_warning_helper(false, msg);
  }
}

void raise_warning_without_first_frame(const std::string &msg) {
  if (warning_freq_check()) {
    raise_warning_helper(true, msg);
  }
}

void raise_warning(const char *fmt, ...) {
  if (!warning_freq_check()) return;
  std::string msg;
  va_list ap;
  va_start(ap, fmt);
  string_vsnprintf(msg, fmt, ap);
  va_end(ap);
  raise_warning_helper(false, msg);
}

static void raise_hack_strict_helper(
  HackStrictOption option, const char *ini_setting, const std::string& msg) {
  if (option == HackStrictOption::WARN) {
    raise_warning_helper(
      false, std::string("(hhvm.hack.") + ini_setting + "=warn) " + msg);
  } else if (option == HackStrictOption::ON) {
    raise_error(std::string("(hhvm.hack.") + ini_setting + "=error) " + msg);
  }
}


/**
 * For use with the HackStrictOption settings. This will warn, error, or do
 * nothing depending on what the user chose for the option. The second param
 * should be the ini setting name after "hhvm.hack."
 */
void raise_hack_strict(HackStrictOption option, const char *ini_setting,
                       const std::string& msg) {
  if (option == HackStrictOption::WARN ?
      !warning_freq_check() : (option != HackStrictOption::ON)) {
    return;
  }
  raise_hack_strict_helper(option, ini_setting, msg);
}

void raise_hack_strict(HackStrictOption option, const char *ini_setting,
                       const char *fmt, ...) {
  if (option == HackStrictOption::WARN ?
      !warning_freq_check() : (option != HackStrictOption::ON)) {
    return;
  }
  std::string msg;
  va_list ap;
  va_start(ap, fmt);
  string_vsnprintf(msg, fmt, ap);
  va_end(ap);
  raise_hack_strict_helper(option, ini_setting, msg);
}

/**
 * Warnings are currently sampled. raise_warning_unsampled can help when
 * migrating warnings to errors.
 *
 * In general, RaiseDebuggingFrequency should be kept at 1.
 */
static int64_t g_raise_warning_unsampled_counter = 0;

void raise_warning_unsampled(const std::string &msg) {
  if (RuntimeOption::RaiseDebuggingFrequency <= 0 ||
      (g_raise_warning_unsampled_counter++) %
      RuntimeOption::RaiseDebuggingFrequency != 0) {
    return;
  }
  if (g_context->errorNeedsHandling(
        static_cast<int>(ErrorMode::WARNING), true,
        ExecutionContext::ErrorThrowMode::Never)) {
    raise_warning_helper(false, msg);
  }
}

void raise_warning_unsampled(const char *fmt, ...) {
  std::string msg;
  va_list ap;
  va_start(ap, fmt);
  string_vsnprintf(msg, fmt, ap);
  va_end(ap);
  raise_warning_unsampled(msg);
}

void raise_notice(const std::string &msg) {
  if (notice_freq_check(ErrorMode::NOTICE)) {
    raise_notice_helper(ErrorMode::NOTICE, false, msg);
  }
}

void raise_notice_without_first_frame(const std::string &msg) {
  if (notice_freq_check(ErrorMode::NOTICE)) {
    raise_notice_helper(ErrorMode::NOTICE, true, msg);
  }
}

void raise_notice(const char *fmt, ...) {
  if (!notice_freq_check(ErrorMode::NOTICE)) return;
  std::string msg;
  va_list ap;
  va_start(ap, fmt);
  string_vsnprintf(msg, fmt, ap);
  va_end(ap);
  raise_notice_helper(ErrorMode::NOTICE, false, msg);
}

void raise_deprecated(const std::string &msg) {
  if (notice_freq_check(ErrorMode::PHP_DEPRECATED)) {
    raise_notice_helper(ErrorMode::PHP_DEPRECATED, false, msg);
  }
}

void raise_deprecated_without_first_frame(const std::string &msg) {
  if (notice_freq_check(ErrorMode::PHP_DEPRECATED)) {
    raise_notice_helper(ErrorMode::PHP_DEPRECATED, true, msg);
  }
}

void raise_deprecated(const char *fmt, ...) {
  if (!notice_freq_check(ErrorMode::PHP_DEPRECATED)) return;
  std::string msg;
  va_list ap;
  va_start(ap, fmt);
  string_vsnprintf(msg, fmt, ap);
  va_end(ap);
  raise_notice_helper(ErrorMode::PHP_DEPRECATED, false, msg);
}

void raise_param_type_warning(
    const char* func_name,
    int param_num,
    DataType expected_type,
    DataType actual_type) {

  // its ok to do this before munging, because it only looks at the
  // end of the string
  auto is_constructor = is_constructor_name(func_name);
  if (!is_constructor && !warning_freq_check()) return;
  // slice off fg1_
  if (strncmp(func_name, "fg1_", 4) == 0) {
    func_name += 4;
  } else if (strncmp(func_name, "tg1_", 4) == 0) {
    func_name += 4;
  } else if (strncmp(func_name, "__SystemLib\\extract", 19) == 0) {
    func_name = "extract";
  } else if (strncmp(func_name, "__SystemLib\\assert", 18) == 0) {
    func_name = "assert";
  } else if (strncmp(func_name, "__SystemLib\\parse_str", 21) == 0) {
    func_name = "parse_str";
  } else if (strncmp(func_name, "__SystemLib\\compact_sl", 22) == 0) {
    func_name = "compact";
  } else if (strncmp(func_name, "__SystemLib\\get_defined_vars", 28) == 0) {
    func_name = "get_defined_vars";
  } else if (strncmp(func_name, "__SystemLib\\func_get_args_sl", 28) == 0) {
    func_name = "func_get_args";
  } else if (strncmp(func_name, "__SystemLib\\func_get_arg_sl", 27) == 0) {
    func_name = "func_get_arg";
  } else if (strncmp(func_name, "__SystemLib\\func_num_arg_", 25) == 0) {
    func_name = "func_num_args";
  }
  assert(param_num > 0);
  auto msg = folly::sformat(
    "{}() expects parameter {} to be {}, {} given",
    func_name,
    param_num,
    getDataTypeString(expected_type).data(),
    getDataTypeString(actual_type).data());

  if (is_constructor) {
    SystemLib::throwExceptionObject(msg);
  }

  raise_warning_helper(false, msg);
}

void raise_message(ErrorMode mode,
                   const char *fmt,
                   va_list ap) {
  std::string msg;
  string_vsnprintf(msg, fmt, ap);
  raise_message(mode, false, msg);
}

void raise_message(ErrorMode mode,
                   const char *fmt,
                   ...) {
  std::string msg;
  va_list ap;
  va_start(ap, fmt);
  string_vsnprintf(msg, fmt, ap);
  va_end(ap);
  raise_message(mode, false, msg);
}

void raise_message(ErrorMode mode,
                   bool skipTop,
                   const std::string &msg) {
  if (mode == ErrorMode::ERROR) {
    HANDLE_ERROR(false, Always, "\nFatal error: ", skipTop);
    not_reached();
  }

  if (mode == ErrorMode::RECOVERABLE_ERROR) {
    HANDLE_ERROR(true, IfUnhandled, "\nCatchable fatal error: ", skipTop);
    return;
  }

  if (!g_context->errorNeedsHandling(static_cast<int>(mode), true,
                                     ExecutionContext::ErrorThrowMode::Never)) {
    return;
  }

  if (mode == ErrorMode::WARNING) {
    if (RuntimeOption::WarningFrequency <= 0 ||
        (g_warning_counter++) % RuntimeOption::WarningFrequency != 0) {
      return;
    }
    HANDLE_ERROR(true, Never, "\nWarning: ", skipTop);
    return;
  }

  if (RuntimeOption::NoticeFrequency <= 0 ||
      (g_notice_counter++) % RuntimeOption::NoticeFrequency != 0) {
    return;
  }

  raise_notice_helper(mode, skipTop, msg);
}

///////////////////////////////////////////////////////////////////////////////
}
