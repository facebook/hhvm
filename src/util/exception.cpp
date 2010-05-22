/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include "exception.h"
#include "base.h"

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Exception::Exception(const char *fmt, ...)
  : m_handled(false) {
  va_list ap; va_start(ap, fmt); format(fmt, ap); va_end(ap);
}

Exception::Exception(bool trace)
  : m_handled(false), m_st(trace) {
}

Exception::Exception(const Exception &e)
  : m_handled(true), m_msg(e.m_msg), m_what(e.m_what), m_st(e.m_st) {
  e.m_handled = true;
}

Exception::Exception()
  : m_handled(true) {
}

void Exception::format(const char *fmt, va_list ap) {
  char buf[20480];
  vsnprintf(buf, sizeof(buf), fmt, ap);
  m_msg = buf;
}

Exception::~Exception() throw() {
}

///////////////////////////////////////////////////////////////////////////////

const char *Exception::what() const throw() {
  if (m_what.empty()) {
    m_what = m_msg + "\n\n" + m_st.toString();
  }
  return m_what.c_str();
}

///////////////////////////////////////////////////////////////////////////////
}
