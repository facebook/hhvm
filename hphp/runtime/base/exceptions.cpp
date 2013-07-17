/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1998-2010 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
*/
#include "hphp/runtime/base/exceptions.h"
#include "hphp/runtime/base/runtime_option.h"
#include "hphp/runtime/base/complex_types.h"
#include "hphp/runtime/base/execution_context.h"

namespace HPHP {

int ExitException::ExitCode = 0;
///////////////////////////////////////////////////////////////////////////////

ExtendedException::ExtendedException() : Exception() {
  computeBacktrace();
}

ExtendedException::ExtendedException(const std::string &msg) {
  m_msg = msg;
  computeBacktrace();
}

ExtendedException::ExtendedException(SkipFrame, const std::string &msg) {
  m_msg = msg;
  computeBacktrace(true);
}

ExtendedException::ExtendedException(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  format(fmt, ap);
  va_end(ap);
  computeBacktrace();
}

ParseTimeFatalException::ParseTimeFatalException(const char* file, int line,
                                                 const char* msg, ...)
  : m_file(file), m_line(line) {
  va_list ap; va_start(ap, msg); format(msg, ap); va_end(ap);
}

FatalErrorException::FatalErrorException(int, const char *msg, ...) {
  va_list ap; va_start(ap, msg); format(msg, ap); va_end(ap);
}

FatalErrorException::FatalErrorException(const std::string &msg,
                                         CArrRef backtrace) {
  m_msg = msg;
  m_btp = backtrace.get();
}

Array ExtendedException::getBackTrace() const {
  return m_btp.get();
}

/**
 * This must be done in the constructor.
 * If you wait too long, getFP() will be NULL.
 */
void ExtendedException::computeBacktrace(bool skipFrame /* = false */) {
  m_btp = g_vmContext->debugBacktrace(skipFrame, true).get();
}

InvalidArgumentException::InvalidArgumentException(int, const char *fmt, ...) {
  va_list ap; va_start(ap, fmt); format(fmt, ap); va_end(ap);
}

void throw_null_pointer_exception() {
  throw NullPointerException();
}

void intrusive_ptr_add_ref(ArrayData* a) {
  a->incRefCount();
}

void intrusive_ptr_release(ArrayData* a) {
  decRefArr(a);
}

///////////////////////////////////////////////////////////////////////////////
}
