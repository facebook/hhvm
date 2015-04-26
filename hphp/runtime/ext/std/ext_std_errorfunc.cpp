/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
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
#include "hphp/runtime/ext/std/ext_std_errorfunc.h"

#include <iostream>

#include <folly/Likely.h>
#include <folly/Format.h>

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/exceptions.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/base/backtrace.h"
#include "hphp/runtime/ext/std/ext_std_file.h"
#include "hphp/util/logger.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const int64_t k_DEBUG_BACKTRACE_PROVIDE_OBJECT = (1 << 0);
const int64_t k_DEBUG_BACKTRACE_IGNORE_ARGS = (1 << 1);
const int64_t k_DEBUG_BACKTRACE_PROVIDE_METADATA = (1 << 16);

const int64_t k_E_ERROR = (1 << 0);
const int64_t k_E_WARNING = (1 << 1);
const int64_t k_E_PARSE = (1 << 2);
const int64_t k_E_NOTICE = (1 << 3);
const int64_t k_E_CORE_ERROR = (1 << 4);
const int64_t k_E_CORE_WARNING = (1 << 5);
const int64_t k_E_COMPILE_ERROR = (1 << 6);
const int64_t k_E_COMPILE_WARNING = (1 << 7);
const int64_t k_E_USER_ERROR = (1 << 8);
const int64_t k_E_USER_WARNING = (1 << 9);
const int64_t k_E_USER_NOTICE = (1 << 10);
const int64_t k_E_STRICT = (1 << 11);
const int64_t k_E_RECOVERABLE_ERROR = (1 << 12);
const int64_t k_E_DEPRECATED = (1 << 13);
const int64_t k_E_USER_DEPRECATED = (1 << 14);
const int64_t k_E_ALL = k_E_ERROR | k_E_WARNING | k_E_PARSE | k_E_NOTICE |
                        k_E_CORE_ERROR | k_E_CORE_WARNING | k_E_COMPILE_ERROR |
                        k_E_COMPILE_WARNING | k_E_USER_ERROR |
                        k_E_USER_WARNING | k_E_USER_NOTICE | k_E_STRICT |
                        k_E_RECOVERABLE_ERROR | k_E_DEPRECATED |
                        k_E_USER_DEPRECATED;

Array HHVM_FUNCTION(debug_backtrace, int64_t options /* = 1 */,
                                     int64_t limit /* = 0 */) {
  bool provide_object = options & k_DEBUG_BACKTRACE_PROVIDE_OBJECT;
  bool provide_metadata = options & k_DEBUG_BACKTRACE_PROVIDE_METADATA;
  bool ignore_args = options & k_DEBUG_BACKTRACE_IGNORE_ARGS;
  return createBacktrace(BacktraceArgs()
                         .withThis(provide_object)
                         .withMetadata(provide_metadata)
                         .ignoreArgs(ignore_args)
                         .setLimit(limit));
}

/**
 * hphp_debug_caller_info - returns an array of info about the "caller"
 *
 * For clarity, we refer to the function that called hphp_debug_caller_info()
 * as the "callee", and we refer to the function that called the callee as
 * the "caller".
 *
 * This function returns an array containing two keys "file" and "line" which
 * indicate the the filename and line number where the "caller" called the
 * "callee".
 */
Array HHVM_FUNCTION(hphp_debug_caller_info) {
  return g_context->getCallerInfo();
}

void HHVM_FUNCTION(debug_print_backtrace, int64_t options /* = 0 */,
                                          int64_t limit /* = 0 */) {
  bool ignore_args = options & k_DEBUG_BACKTRACE_IGNORE_ARGS;
  g_context->write(debug_string_backtrace(false, ignore_args, limit));
}

const StaticString
  s_class("class"),
  s_type("type"),
  s_function("function"),
  s_file("file"),
  s_line("line"),
  s_message("message"),
  s_args("args");

String debug_string_backtrace(bool skip, bool ignore_args /* = false */,
                              int64_t limit /* = 0 */) {
  Array bt;
  StringBuffer buf;
  bt = createBacktrace(BacktraceArgs()
                       .skipTop(skip)
                       .ignoreArgs(ignore_args)
                       .setLimit(limit));
  int i = 0;
  for (ArrayIter it = bt.begin(); !it.end(); it.next(), i++) {
    Array frame = it.second().toArray();
    buf.append('#');
    buf.append(i);
    if (i < 10) buf.append(' ');
    buf.append(' ');
    if (frame.exists(s_class)) {
      buf.append(frame->get(s_class).toString());
      buf.append(frame->get(s_type).toString());
    }
    buf.append(frame->get(s_function).toString());
    buf.append("(");
    if (!ignore_args) {
      bool first = true;
      for (ArrayIter it(frame->get(s_args).toArray());
          !it.end();
          it.next()) {
        if (!first) {
          buf.append(", ");
        } else {
          first = false;
        }
        try {
          buf.append(it.second().toString());
        } catch (FatalErrorException& fe) {
          buf.append(fe.getMessage());
        }
      }
    }
    buf.append(")");
    if (frame.exists(s_file)) {
      buf.append(" called at [");
      buf.append(frame->get(s_file).toString());
      buf.append(':');
      buf.append(frame->get(s_line).toString());
      buf.append(']');
    }
    buf.append('\n');
  }
  return buf.detach();
}

Array HHVM_FUNCTION(error_get_last) {
  String lastError = g_context->getLastError();
  if (lastError.isNull()) {
    return Array();
  }
  return make_map_array(s_type, g_context->getLastErrorNumber(),
                        s_message, g_context->getLastError(),
                        s_file, g_context->getLastErrorPath(),
                        s_line, g_context->getLastErrorLine());
}

bool HHVM_FUNCTION(error_log, const String& message, int message_type /* = 0 */,
                              const Variant& destination /* = null */,
                              const Variant& extra_headers /* = null */) {
  // error_log() should not invoke the user error handler,
  // so we use Logger::Error() instead of raise_warning() or raise_error()
  switch (message_type) {
  case 0:
  {
    std::string line(message.data(),
                     // Truncate to 512k
                     message.size() > (1<<19) ? (1<<19) : message.size());
    Logger::Error(line);
    return true;
  }
  case 3:
  {
    // open for append only
    auto outfile = HHVM_FN(fopen)(destination.toString(), "a");
    if (outfile.isNull()) {
      Logger::Error("can't open error_log file!\n");
      return false;
    }
    HHVM_FN(fwrite)(outfile.toResource(), message);
    HHVM_FN(fclose)(outfile.toResource());
    return true;
  }
  case 2: // not used per PHP
  default:
    Logger::Error("error_log does not support message_type %d!", message_type);
    break;
  }
  return false;
}

int64_t HHVM_FUNCTION(error_reporting, const Variant& level /* = null */) {
  auto& id = ThreadInfo::s_threadInfo.getNoCheck()->m_reqInjectionData;
  int oldErrorReportingLevel = id.getErrorReportingLevel();
  if (!level.isNull()) {
    id.setErrorReportingLevel(level.toInt32());
  }
  return oldErrorReportingLevel;
}

bool HHVM_FUNCTION(restore_error_handler) {
  g_context->popUserErrorHandler();
  return true;
}

bool HHVM_FUNCTION(restore_exception_handler) {
  g_context->popUserExceptionHandler();
  return true;
}

Variant HHVM_FUNCTION(set_error_handler, const Variant& error_handler,
                                         int error_types /* = k_E_ALL */) {
  return g_context->pushUserErrorHandler(error_handler, error_types);
}

Variant HHVM_FUNCTION(set_exception_handler, const Variant& exception_handler) {
  return g_context->pushUserExceptionHandler(exception_handler);
}

void HHVM_FUNCTION(hphp_set_error_page, const String& page) {
  g_context->setErrorPage(page);
}

void HHVM_FUNCTION(hphp_throw_fatal_error, const String& error_msg) {
  std::string msg = error_msg.data();
  raise_error(msg);
}

void HHVM_FUNCTION(hphp_clear_unflushed) {
  g_context->obEndAll();
  g_context->obStart();
  g_context->obProtect(true);
}

bool HHVM_FUNCTION(trigger_error, const String& error_msg,
                                  int error_type /* = k_E_USER_NOTICE */) {
  std::string msg = error_msg.data();
  if (UNLIKELY(g_context->getThrowAllErrors())) {
    throw Exception(folly::sformat("throwAllErrors: {}", error_type));
  }
  if (error_type == k_E_USER_ERROR) {
    g_context->handleError(msg, error_type, true,
                       ExecutionContext::ErrorThrowMode::IfUnhandled,
                       "\nFatal error: ");
  } else if (error_type == k_E_USER_WARNING) {
    g_context->handleError(msg, error_type, true,
                       ExecutionContext::ErrorThrowMode::Never,
                       "\nWarning: ");
  } else if (error_type == k_E_USER_NOTICE) {
    g_context->handleError(msg, error_type, true,
                       ExecutionContext::ErrorThrowMode::Never,
                       "\nNotice: ");
  } else if (error_type == k_E_USER_DEPRECATED) {
    g_context->handleError(msg, error_type, true,
                       ExecutionContext::ErrorThrowMode::Never,
                       "\nDeprecated: ");
  } else {
    ActRec* fp = g_context->getStackFrame();
    if (fp->m_func->isBuiltin() && error_type == k_E_ERROR) {
      raise_error_without_first_frame(msg);
    } else if (fp->m_func->isBuiltin() && error_type == k_E_WARNING) {
      raise_warning_without_first_frame(msg);
    } else if (fp->m_func->isBuiltin() && error_type == k_E_NOTICE) {
      raise_notice_without_first_frame(msg);
    } else if (fp->m_func->isBuiltin() && error_type == k_E_DEPRECATED) {
      raise_deprecated_without_first_frame(msg);
    } else if (fp->m_func->isBuiltin() && error_type == k_E_RECOVERABLE_ERROR) {
      raise_recoverable_error_without_first_frame(msg);
    } else {
    raise_warning("Invalid error type specified");
    return false;
    }
  }
  return true;
}

bool HHVM_FUNCTION(user_error, const String& error_msg,
                               int error_type /* = k_E_USER_NOTICE */) {
  return HHVM_FN(trigger_error)(error_msg, error_type);
}

///////////////////////////////////////////////////////////////////////////////

void StandardExtension::initErrorFunc() {
  HHVM_FE(debug_backtrace);
  HHVM_FE(hphp_debug_caller_info);
  HHVM_FE(debug_print_backtrace);
  HHVM_FE(error_get_last);
  HHVM_FE(error_log);
  HHVM_FE(error_reporting);
  HHVM_FE(restore_error_handler);
  HHVM_FE(restore_exception_handler);
  HHVM_FE(set_error_handler);
  HHVM_FE(set_exception_handler);
  HHVM_FE(hphp_set_error_page);
  HHVM_FE(hphp_throw_fatal_error);
  HHVM_FE(hphp_clear_unflushed);
  HHVM_FE(trigger_error);
  HHVM_FE(user_error);

#define INTCONST(v) Native::registerConstant<KindOfInt64> \
                  (makeStaticString(#v), k_##v);
  INTCONST(DEBUG_BACKTRACE_PROVIDE_OBJECT);
  INTCONST(DEBUG_BACKTRACE_IGNORE_ARGS);
  INTCONST(DEBUG_BACKTRACE_PROVIDE_METADATA);
  INTCONST(E_ERROR);
  INTCONST(E_WARNING);
  INTCONST(E_PARSE);
  INTCONST(E_NOTICE);
  INTCONST(E_CORE_ERROR);
  INTCONST(E_CORE_WARNING);
  INTCONST(E_COMPILE_ERROR);
  INTCONST(E_COMPILE_WARNING);
  INTCONST(E_USER_ERROR);
  INTCONST(E_USER_WARNING);
  INTCONST(E_USER_NOTICE);
  INTCONST(E_STRICT);
  INTCONST(E_RECOVERABLE_ERROR);
  INTCONST(E_DEPRECATED);
  INTCONST(E_USER_DEPRECATED);
  INTCONST(E_ALL);
#undef INTCONST

  loadSystemlib("std_errorfunc");
}

}
