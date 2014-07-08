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
#include "hphp/runtime/base/exceptions.h"

#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/execution-context.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

std::atomic<int> ExitException::ExitCode{0};  // XXX: this should not be static

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

Array ExtendedException::getBackTrace() const {
  return Array(m_btp.get());
}

const StaticString s_file("file"), s_line("line");

std::pair<String, int> ExtendedException::getFileAndLine() const {
  String file = empty_string();
  int line = 0;
  Array bt = getBackTrace();
  if (!bt.empty()) {
    Array top = bt.rvalAt(0).toArray();
    if (top.exists(s_file)) file = top.rvalAt(s_file).toString();
    if (top.exists(s_line)) line = top.rvalAt(s_line).toInt64();
  }
  return std::make_pair(file, line);
}

void ExtendedException::computeBacktrace(bool skipFrame /* = false */) {
  m_btp = g_context->debugBacktrace(skipFrame, true).get();
}

//////////////////////////////////////////////////////////////////////

ParseTimeFatalException::ParseTimeFatalException(const char* file, int line,
                                                 const char* msg, ...)
  : m_file(file), m_line(line) {
  va_list ap; va_start(ap, msg); format(msg, ap); va_end(ap);
}

FatalErrorException::FatalErrorException(int, const char *msg, ...) {
  va_list ap; va_start(ap, msg); format(msg, ap); va_end(ap);
}

FatalErrorException::FatalErrorException(const std::string& msg,
                                         const Array& backtrace)
  : ExtendedException(msg, backtrace.get())
{}

//////////////////////////////////////////////////////////////////////

void throw_null_pointer_exception() {
  throw ExtendedException("A null object pointer was used.");
}

void throw_invalid_object_type(const char* clsName) {
  throw ExtendedException("Unexpected object type %s.", clsName);
}

void throw_not_implemented(const char* feature) {
  throw ExtendedException("%s is not implemented yet.", feature);
}

void throw_not_supported(const char* feature, const char* reason) {
  throw ExtendedException("%s is not supported: %s", feature, reason);
}

void intrusive_ptr_add_ref(ArrayData* a) {
  a->incRefCount();
}

void intrusive_ptr_release(ArrayData* a) {
  decRefArr(a);
}

///////////////////////////////////////////////////////////////////////////////
}
