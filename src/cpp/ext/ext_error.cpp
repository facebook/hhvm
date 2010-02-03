/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <cpp/ext/ext_error.h>
#include <cpp/base/util/exceptions.h>
#include <cpp/base/runtime_option.h>
#include <cpp/base/util/string_buffer.h>
#include <cpp/base/source_info.h>
#include <cpp/base/debug/backtrace.h>
#include <cpp/base/frame_injection.h>
#include <lib/system/gen/php/globals/constants.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Array f_debug_backtrace() {
  if (RuntimeOption::InjectedStacktrace) {
    return FrameInjection::getBacktrace(true);
  } else {
    StackTrace st;
    return stackTraceToBackTrace(st);
  }
}

void f_debug_print_backtrace() {
  if (RuntimeOption::InjectedStacktrace) {
    Array bt = FrameInjection::getBacktrace(true);
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
  } else {
    return CREATE_VECTOR1(g_context->getLastError());
  }
}

bool f_error_log(CStrRef message, int message_type /* = 0 */,
                 CStrRef destination /* = null_string */,
                 CStrRef extra_headers /* = null_string */) {
  String line = message.replace("\n", "\\n");
  Logger::Error("%s", line.data());
  return true;
}

int f_error_reporting(int level /* = 0 */) {
  int oldErrorReportingLevel = g_context->getErrorReportingLevel();
  g_context->setErrorReportingLevel(level);
  return oldErrorReportingLevel;
}

bool f_restore_error_handler() {
  g_context->popSystemExceptionHandler();
  return true;
}

bool f_restore_exception_handler() {
  g_context->popUserExceptionHandler();
  return false;
}

Variant f_set_error_handler(CStrRef error_handler, int error_types /* = 0 */) {
  return g_context->pushSystemExceptionHandler(error_handler);
}

String f_set_exception_handler(CStrRef exception_handler) {
  return g_context->pushUserExceptionHandler(exception_handler);
}

bool f_trigger_error(CStrRef error_msg,
                     int error_type /* = k_E_USER_NOTICE */) {
  Logger::LogLevelType level;
  if (error_type == k_E_USER_ERROR) {
    level = Logger::LogError;
  } else if (error_type == k_E_USER_WARNING) {
    level = Logger::LogWarning;
  } else if (error_type == k_E_USER_NOTICE) {
    level = Logger::LogVerbose;
  } else {
    return false;
  }
  if (Logger::LogLevel <= level) {
    throw Exception("Error %d: %s", error_type, (const char *)error_msg);
  }
  return true;
}

bool f_user_error(CStrRef error_msg, int error_type /* = k_E_USER_NOTICE */) {
  return f_trigger_error(error_msg, error_type);
}

///////////////////////////////////////////////////////////////////////////////
}
