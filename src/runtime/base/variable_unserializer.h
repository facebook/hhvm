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

#ifndef __HPHP_VARIABLE_UNSERIALIZER_H__
#define __HPHP_VARIABLE_UNSERIALIZER_H__

#include <runtime/base/types.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class VariableUnserializer {
public:
  /**
   * Supported formats.
   */
  enum Type {
    Serialize,
    APCSerialize,
  };

public:
  VariableUnserializer(const char *str, size_t len, Type type,
                       bool allowUnknownSerializableClass = false)
      : m_type(type), m_buf(str), m_end(str + len), m_key(false),
        m_unknownSerializable(allowUnknownSerializableClass) {}
  VariableUnserializer(const char *str, const char *end, Type type,
                       bool allowUnknownSerializableClass = false)
      : m_type(type), m_buf(str), m_end(end), m_key(false),
        m_unknownSerializable(allowUnknownSerializableClass) {}

  Type getType() const { return m_type;}
  bool allowUnknownSerializableClass() const { return m_unknownSerializable;}

  Variant unserialize();
  Variant unserializeKey();
  void add(Variant* v) {
    if (!m_key) {
      m_refs.push_back(v);
    }
  }
  Variant *get(int id) {
    if (id <= 0  || id > (int)m_refs.size()) return NULL;
    return m_refs[id-1];
  }
  int64 readInt();
  double readDouble();
  char readChar() {
    check();
    return *(m_buf++);
  }
  void read(char *buf, uint n);
  char peek() {
    check();
    return *m_buf;
  }
  const char *head() { return m_buf; }

 private:
  Type m_type;
  const char *m_buf;
  const char *m_end;
  std::vector<Variant*> m_refs;
  bool m_key;
  bool m_unknownSerializable;

  void check() {
    if (m_buf >= m_end) {
      throw Exception("Unexpected end of buffer during unserialization");
    }
  }
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_VARIABLE_UNSERIALIZER_H__
