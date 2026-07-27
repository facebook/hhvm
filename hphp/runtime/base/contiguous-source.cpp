/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/base/contiguous-source.h"

#include <algorithm>

#include "hphp/util/exception.h"
#include "hphp/util/portability.h"

#include "hphp/zend/zend-strtod.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace detail {

[[noreturn]] NEVER_INLINE
void throwUnexpectedEOB() {
  throw Exception("Unexpected end of buffer during unserialization");
}

[[noreturn]] NEVER_INLINE
void throwUnexpectedSep(char expect, char actual) {
  throw Exception("Expected '%c' but got '%c'", expect, actual);
}

}

int64_t ContiguousSource::readInt() {
  check();
  auto const r = hh_strtoll_base10(m_buf);
  m_buf = r.second;
  return r.first;
}

double ContiguousSource::readDouble() {
  check();
  const char* newBuf;
  auto const r = zend_strtod(m_buf, &newBuf);
  m_buf = newBuf;
  return r;
}

folly::StringPiece ContiguousSource::readStr(unsigned n) {
  check();
  auto const bufferLimit = std::min(size_t(m_end - m_buf), size_t(n));
  auto const str = folly::StringPiece(m_buf, bufferLimit);
  m_buf += bufferLimit;
  return str;
}

///////////////////////////////////////////////////////////////////////////////
}
