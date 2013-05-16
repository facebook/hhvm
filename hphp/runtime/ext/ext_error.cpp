/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include "hphp/runtime/ext/ext_error.h"
#include "hphp/runtime/base/util/exceptions.h"
#include "hphp/runtime/base/runtime_option.h"
#include "hphp/runtime/base/util/string_buffer.h"
#include "hphp/util/logger.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Array f_debug_backtrace(bool provide_object /* = true */) {
  return g_vmContext->debugBacktrace(true, false, provide_object);
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
Array f_hphp_debug_caller_info() {
  if (RuntimeOption::InjectedStackTrace) {
    return g_vmContext->getCallerInfo();
  }
  return Array::Create();
}

void f_debug_print_backtrace() {
  echo(debug_string_backtrace(true));
}

static const StaticString s_class("class");
static const StaticString s_type("type");
static const StaticString s_function("function");
static const StaticString s_file("file");
static const StaticString s_line("line");
static const StaticString s_message("message");

String debug_string_backtrace(bool skip) {
  if (RuntimeOption::InjectedStackTrace) {
    Array bt;
    StringBuffer buf;
    bt = g_vmContext->debugBacktrace(skip);
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
      buf.append("()");
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
  } else {
    StackTrace st;
    return String(st.toString());
  }
}

Array f_error_get_last() {
  String lastError = g_context->getLastError();
  if (lastError.isNull()) {
    return (ArrayData *)NULL;
  }
  return CREATE_MAP2(s_message, g_context->getLastError(),
                     s_type, g_context->getLastErrorNumber());
}

bool f_error_log(CStrRef message, int message_type /* = 0 */,
                 CStrRef destination /* = null_string */,
                 CStrRef extra_headers /* = null_string */) {
  // error_log() should not invoke the user error handler,
  // so we use Logger::Error() instead of raise_warning()
  std::string line(message.data(),
                   // Truncate to 512k
                   message.size() > (1<<19) ? (1<<19) : message.size());
  if (RuntimeOption::serverExecutionMode() ||
      RuntimeOption::AlwaysEscapeLog) {
    Logger::Error(line);
  } else {
    Logger::RawError(line);

    // otherwise errors will go to error log without displaying on screen
    if (Logger::UseLogFile && Logger::Output) {
      std::cerr << line;
    }
  }
  return true;
}

int64_t f_error_reporting(CVarRef level /* = null */) {
  int oldErrorReportingLevel = g_context->getErrorReportingLevel();
  if (!level.isNull()) {
    g_context->setErrorReportingLevel(level.toInt32());
  }
  return oldErrorReportingLevel;
}

bool f_restore_error_handler() {
  g_context->popUserErrorHandler();
  return true;
}

bool f_restore_exception_handler() {
  g_context->popUserExceptionHandler();
  return false;
}

Variant f_set_error_handler(CVarRef error_handler,
                            int error_types /* = k_E_ALL */) {
  return g_context->pushUserErrorHandler(error_handler, error_types);
}

Variant f_set_exception_handler(CVarRef exception_handler) {
  return g_context->pushUserExceptionHandler(exception_handler);
}

void f_hphp_set_error_page(CStrRef page) {
  g_context->setErrorPage(page);
}

void f_hphp_throw_fatal_error(CStrRef error_msg) {
  std::string msg = error_msg.data();
  raise_error(msg);
}

void f_hphp_clear_unflushed() {
  g_context->obEndAll();
  g_context->obStart();
  g_context->obProtect(true);
}

bool f_trigger_error(CStrRef error_msg,
                     int error_type /* = k_E_USER_NOTICE */) {
  std::string msg = error_msg.data();
  if (error_type == k_E_USER_ERROR) {
    g_context->handleError(msg, error_type, true,
                       ExecutionContext::ThrowIfUnhandled,
                       "HipHop Recoverable error: ");
  } else if (error_type == k_E_USER_WARNING) {
    g_context->handleError(msg, error_type, true,
                       ExecutionContext::NeverThrow,
                       "HipHop Warning: ");
  } else if (error_type == k_E_USER_NOTICE) {
    g_context->handleError(msg, error_type, true,
                       ExecutionContext::NeverThrow,
                       "HipHop Notice: ");
  } else if (error_type == k_E_USER_DEPRECATED) {
    g_context->handleError(msg, error_type, true,
                       ExecutionContext::NeverThrow,
                       "HipHop Deprecated: ");
  } else {
    return false;
  }
  return true;
}

bool f_user_error(CStrRef error_msg, int error_type /* = k_E_USER_NOTICE */) {
  return f_trigger_error(error_msg, error_type);
}

///////////////////////////////////////////////////////////////////////////////
}
