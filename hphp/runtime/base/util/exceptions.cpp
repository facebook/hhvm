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
#include <runtime/base/complex_types.h>
#include <runtime/base/execution_context.h>

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
