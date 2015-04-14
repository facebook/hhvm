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
#ifndef incl_HPHP_VARIABLE_UNSERIALIZER_INL_H
#define incl_HPHP_VARIABLE_UNSERIALIZER_INL_H

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

inline VariableUnserializer::RefInfo::RefInfo(Variant* v)
    : m_data(reinterpret_cast<uintptr_t>(v))
{}

inline VariableUnserializer::RefInfo
VariableUnserializer::RefInfo::makeNonRefable(Variant* v) {
  RefInfo r(v);
  r.m_data |= 1;
  return r;
}

inline Variant* VariableUnserializer::RefInfo::var() const  {
  return reinterpret_cast<Variant*>(m_data & ~1);
}

inline bool VariableUnserializer::RefInfo::canBeReferenced() const {
  return !(m_data & 1);
}

///////////////////////////////////////////////////////////////////////////////

inline VariableUnserializer::VariableUnserializer(
  const char* str,
  size_t len,
  Type type,
  bool allowUnknownSerializableClass,
  const Array& classWhitelist)
    : m_type(type)
    , m_buf(str)
    , m_end(str + len)
    , m_unknownSerializable(allowUnknownSerializableClass)
    , m_classWhiteList(classWhitelist)
{}

inline VariableUnserializer::Type VariableUnserializer::type() const {
  return m_type;
}

inline bool VariableUnserializer::allowUnknownSerializableClass() const {
  return m_unknownSerializable;
}

inline const char* VariableUnserializer::head() const {
  return m_buf;
}

inline char VariableUnserializer::peek() const {
  check();
  return *m_buf;
}

inline char VariableUnserializer::peekBack() const {
  return m_buf[-1];
}

inline bool VariableUnserializer::endOfBuffer() const {
  return m_buf >= m_end;
}

inline char VariableUnserializer::readChar() {
  check();
  return *(m_buf++);
}

inline void VariableUnserializer::expectChar(char expected) {
  char ch = readChar();
  if (UNLIKELY(ch != expected)) {
    throwUnexpected(expected, ch);
  }
}

inline void VariableUnserializer::add(Variant* v, UnserializeMode mode) {
  if (mode == UnserializeMode::Value) {
    m_refs.emplace_back(RefInfo(v));
  } else if (mode == UnserializeMode::Key) {
    // do nothing
  } else if (mode == UnserializeMode::ColValue) {
    m_refs.emplace_back(RefInfo::makeNonRefable(v));
  } else {
    assert(mode == UnserializeMode::ColKey);
    // We don't currently support using the 'r' encoding to refer
    // to collection keys, but eventually we'll need to make this
    // work to allow objects as keys. For now we encode collections
    // keys in m_refs using a null pointer.
    m_refs.emplace_back(RefInfo(nullptr));
  }
}

inline Variant* VariableUnserializer::getByVal(int id) {
  if (id <= 0 || id > (int)m_refs.size()) return nullptr;
  Variant* ret = m_refs[id-1].var();
  if (!ret) {
    throw Exception("Referring to collection keys using the 'r' encoding "
                    "is not supported");
  }
  return ret;
}

inline Variant* VariableUnserializer::getByRef(int id) {
  if (id <= 0 || id > (int)m_refs.size()) return nullptr;
  if (!m_refs[id-1].canBeReferenced()) {
    // If the low bit is set, that means the value cannot
    // be taken by reference
    throw Exception("Collection values cannot be taken by reference");
  }
  Variant* ret = m_refs[id-1].var();
  if (!ret) {
    throw Exception("Collection keys cannot be taken by reference");
  }
  return ret;
}

inline void VariableUnserializer::check() const {
  if (m_buf >= m_end) {
    throw Exception("Unexpected end of buffer during unserialization");
  }
}

///////////////////////////////////////////////////////////////////////////////

}

#endif
