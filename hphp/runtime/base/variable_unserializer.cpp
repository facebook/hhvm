/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include <runtime/base/variable_unserializer.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/zend/zend_strtod.h>
#include <runtime/base/array/array_iterator.h>
#include <runtime/ext/ext_class.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Variant VariableUnserializer::unserialize() {
  Variant v;
  v.unserialize(this);
  return v;
}

Variant VariableUnserializer::unserializeKey() {
  Variant v;
  v.unserialize(this, Uns::KeyMode);
  return v;
}

int64_t VariableUnserializer::readInt() {
  check();
  char *newBuf;
  int64_t r = strtoll(m_buf, &newBuf, 10);
  m_buf = newBuf;
  return r;
}

double VariableUnserializer::readDouble() {
  check();
  char *newBuf;
  double r = zend_strtod(m_buf, &newBuf);
  m_buf = newBuf;
  return r;
}

void VariableUnserializer::read(char *buf, uint n) {
  check();

  /* compute copy boundaries in a more efficient manner,
     by using min(...) operation rather than complex conditional
     in a loop guard */
  const size_t BUFFER_SIZE = m_end - m_buf;
  const size_t BUFFER_LIMIT = std::min(BUFFER_SIZE, size_t(n));

  memcpy(buf, m_buf, BUFFER_LIMIT);
  m_buf += BUFFER_LIMIT;
}

Variant &VariableUnserializer::addVar() {
  m_vars.push_back(uninit_null());
  return m_vars.back();
}

bool VariableUnserializer::isWhitelistedClass(CStrRef cls_name) const {
  if (m_type != Serialize || m_classWhiteList.isNull()) {
    return true;
  }
  if (!m_classWhiteList.isNull() && !m_classWhiteList.empty()) {
    for (ArrayIter iter(m_classWhiteList); iter; ++iter) {
      CVarRef value(iter.secondRef());
      if (f_is_subclass_of(cls_name, value) || value.same(cls_name)) {
        return true;
      }
    }
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
}
