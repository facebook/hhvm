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
#include <runtime/base/util/exceptions.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/frame_injection.h>
#include <runtime/base/complex_types.h>

namespace HPHP {

int ExitException::ExitCode = 0;
///////////////////////////////////////////////////////////////////////////////

ExtendedException::ExtendedException() : Exception() {
  if (RuntimeOption::InjectedStackTrace) {
    m_bt = ArrayPtr(new Array(FrameInjection::GetBacktrace(false, true)));
  }
}

ExtendedException::ExtendedException(const std::string &msg) {
  m_msg = msg;
  if (RuntimeOption::InjectedStackTrace) {
    m_bt = ArrayPtr(new Array(FrameInjection::GetBacktrace(false, true)));
  }
}

ExtendedException::ExtendedException(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  format(fmt, ap);
  va_end(ap);
  if (RuntimeOption::InjectedStackTrace) {
    m_bt = ArrayPtr(new Array(FrameInjection::GetBacktrace(false, true)));
  }
}

FatalErrorException::FatalErrorException(const std::string &msg,
                                         ArrayPtr backtrace) {
  m_msg = msg;
  m_bt = backtrace;
}

void throw_null_pointer_exception() {
  throw NullPointerException();
}

///////////////////////////////////////////////////////////////////////////////
}
