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
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/repo-global-data.h"
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
  raise_message(ErrorMode::ERROR, false, msg);
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

/*
 * This does not call raise_message because the mode of the error differs
 * depending on the runtime option unlike the regular raise_error.
 */
void raise_error_without_first_frame(const std::string &msg) {
  g_context->handleError(msg, static_cast<int>(ErrorMode::ERROR), false,
                         RuntimeOption::CallUserHandlerOnFatals ?
                         ExecutionContext::ErrorThrowMode::IfUnhandled :
                         ExecutionContext::ErrorThrowMode::Always,
                         "\nFatal error: ",
                         true);
  always_assert(0);
}

void raise_recoverable_error(const std::string &msg) {
  raise_message(ErrorMode::RECOVERABLE_ERROR, false, msg);
}

void raise_recoverable_error_without_first_frame(const std::string &msg) {
  raise_message(ErrorMode::RECOVERABLE_ERROR, true, msg);
}

void raise_typehint_error(const std::string& msg) {
  raise_recoverable_error(msg);
  if (RuntimeOption::RepoAuthoritative && Repo::global().HardTypeHints) {
    raise_error("Error handler tried to recover from typehint violation");
  }
}

void raise_return_typehint_error(const std::string& msg) {
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

void raise_strict_warning(const std::string &msg) {
  raise_message(ErrorMode::STRICT, false, msg);
}

void raise_strict_warning_without_first_frame(const std::string &msg) {
  raise_message(ErrorMode::STRICT, true, msg);
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
  raise_message(ErrorMode::WARNING, false, msg);
}

void raise_warning_without_first_frame(const std::string &msg) {
  raise_message(ErrorMode::WARNING, true, msg);
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
  } else if (option == HackStrictOption::ON) {
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
  raise_message(ErrorMode::WARNING, false, msg);
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
  raise_message(ErrorMode::NOTICE, false, msg);
}

void raise_notice_without_first_frame(const std::string &msg) {
  raise_message(ErrorMode::NOTICE, true, msg);
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
  raise_message(ErrorMode::PHP_DEPRECATED, false, msg);
}

void raise_deprecated_without_first_frame(const std::string &msg) {
  raise_message(ErrorMode::PHP_DEPRECATED, true, msg);
}

void raise_deprecated(const char *fmt, ...) {
  std::string msg;
  va_list ap;
  va_start(ap, fmt);
  string_vsnprintf(msg, fmt, ap);
  va_end(ap);
  raise_message(ErrorMode::PHP_DEPRECATED, false, msg);
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

  if (is_constructor_name(func_name)) {
    SystemLib::throwExceptionObject(msg);
  }

  raise_warning(msg);
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

#define HANDLE_ERROR(userHandle, mode, str, skip)                       \
  g_context->handleError(msg, errnum, userHandle,                       \
                         ExecutionContext::ErrorThrowMode::mode,        \
                         str,                                           \
                         skip);

void raise_message(ErrorMode mode,
                   bool skipTop,
                   const std::string &msg) {
  int errnum = static_cast<int>(mode);
  if (mode == ErrorMode::ERROR) {
    HANDLE_ERROR(false, Always, "\nFatal error: ", skipTop);
  } else if (mode == ErrorMode::RECOVERABLE_ERROR) {
    HANDLE_ERROR(true, IfUnhandled, "\nCatchable fatal error: ", skipTop);
  } else if (!g_context->errorNeedsHandling(errnum, true,
                              ExecutionContext::ErrorThrowMode::Never)) {
    return;
  } else if (mode == ErrorMode::WARNING) {
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
}
///////////////////////////////////////////////////////////////////////////////
}
