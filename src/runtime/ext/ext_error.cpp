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

#include <runtime/ext/ext_error.h>
#include <runtime/base/util/exceptions.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/util/string_buffer.h>
#include <runtime/base/source_info.h>
#include <runtime/base/debug/backtrace.h>
#include <runtime/base/frame_injection.h>
#include <system/gen/php/globals/constants.h>
#include <util/logger.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Array f_debug_backtrace(bool provide_object /* = true */) {
  if (RuntimeOption::InjectedStackTrace) {
    return FrameInjection::GetBacktrace(true, false, provide_object);
  }
  StackTrace st;
  return stackTraceToBackTrace(st);
}

/**
 * hphp_debug_caller_info - returns an array of info about the "caller"
 *
 * For clarity, we refer to the function that called debug_get_caller_info()
 * as the "callee", and we refer to the function that called the callee as
 * the "caller".
 *
 * This function returns an array containing two keys "file" and "line" which
 * indicate the the filename and line number where the "caller" called the
 * "callee".
 */
Array f_hphp_debug_caller_info() {
  if (RuntimeOption::InjectedStackTrace) {
    return FrameInjection::GetCallerInfo(true);
  }
  return Array::Create();
}

void f_debug_print_backtrace() {
  if (RuntimeOption::InjectedStackTrace) {
    Array bt = FrameInjection::GetBacktrace(true);
    int i = 0;
    for (ArrayIter it = bt.begin(); !it.end(); it.next(), i++) {
      Array frame = it.second().toArray();
      StringBuffer buf;
      buf.append('#');
      buf.append(i);
      if (i < 10) buf.append(' ');
      buf.append(' ');
      if (frame.exists("class")) {
        buf.append(frame->get("class").toString());
        buf.append(frame->get("type").toString());
      }
      buf.append(frame->get("function").toString());
      buf.append("()");
      if (frame.exists("file")) {
        buf.append(" called at [");
        buf.append(frame->get("file").toString());
        buf.append(':');
        buf.append(frame->get("line").toString());
        buf.append(']');
      }
      buf.append('\n');
      echo(buf.detach());
    }
  } else {
    StackTrace st;
    echo(String(st.toString()));
  }
}

Array f_error_get_last() {
  String lastError = g_context->getLastError();
  if (lastError.isNull()) {
    return (ArrayData *)NULL;
  }
  return CREATE_MAP2("message", g_context->getLastError(),
                     "type", g_context->getLastErrorNumber());
}

bool f_error_log(CStrRef message, int message_type /* = 0 */,
                 CStrRef destination /* = null_string */,
                 CStrRef extra_headers /* = null_string */) {
  // error_log() should not invoke the user error handler,
  // so we use Logger::Error() instead of raise_warning()
  std::string line(message.data(),
                   // Truncate to 512k
                   message.size() > (1<<19) ? (1<<19) : message.size());
  if (strcmp(RuntimeOption::ExecutionMode, "srv") == 0 ||
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

int f_error_reporting(CVarRef level /* = null */) {
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

String f_set_exception_handler(CVarRef exception_handler) {
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
                       "HipHop Warning:  ");
  } else if (error_type == k_E_USER_NOTICE) {
    g_context->handleError(msg, error_type, true,
                       ExecutionContext::NeverThrow,
                       "HipHop Notice:  ");
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
