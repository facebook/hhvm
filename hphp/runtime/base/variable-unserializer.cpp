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
#include "hphp/runtime/base/variable-unserializer.h"

#include <algorithm>
#include <utility>

#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/zend-strtod.h"
#include "hphp/runtime/ext/std/ext_std_classobj.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

void VariableUnserializer::set(const char* buf, const char* end) {
  m_buf = buf;
  m_end = end;
}

Variant VariableUnserializer::unserialize() {
  Variant v;
  unserializeVariant(v, this);
  return v;
}

static std::pair<int64_t,const char*> hh_strtoll_base10(const char* p) {
  int64_t x = 0;
  bool neg = false;
  if (*p == '-') {
    neg = true;
    ++p;
  }
  while (*p >= '0' && *p <= '9') {
    x = (x * 10) + ('0' - *p);
    ++p;
  }
  if (!neg) {
    x = -x;
  }
  return std::pair<int64_t,const char*>(x, p);
}

int64_t VariableUnserializer::readInt() {
  check();
  auto r = hh_strtoll_base10(m_buf);
  m_buf = r.second;
  return r.first;
}

double VariableUnserializer::readDouble() {
  check();
  const char* newBuf;
  double r = zend_strtod(m_buf, &newBuf);
  m_buf = newBuf;
  return r;
}

void VariableUnserializer::read(char* buf, unsigned n) {
  check();
  auto const bufferLimit = std::min(size_t(m_end - m_buf), size_t(n));
  memcpy(buf, m_buf, bufferLimit);
  m_buf += bufferLimit;
}

void VariableUnserializer::throwUnexpected(char expected, char got) {
  throw Exception("Expected '%c' but got '%c'", expected, got);
}

bool VariableUnserializer::isWhitelistedClass(const String& clsName) const {
  if (m_type != Type::Serialize || m_classWhiteList.isNull()) {
    return true;
  }
  if (!m_classWhiteList.isNull() && !m_classWhiteList.empty()) {
    for (ArrayIter iter(m_classWhiteList); iter; ++iter) {
      const Variant& value(iter.secondRef());
      if (HHVM_FN(is_subclass_of)(clsName, value.toString()) ||
          same(value, clsName)) {
        return true;
      }
    }
  }
  return false;
}

void VariableUnserializer::putInOverwrittenList(const Variant& v) {
  m_overwrittenList.append(v);
}

///////////////////////////////////////////////////////////////////////////////
}
