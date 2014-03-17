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

#ifndef incl_HPHP_VARIABLE_UNSERIALIZER_H_
#define incl_HPHP_VARIABLE_UNSERIALIZER_H_

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/smart-containers.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class VariableUnserializer {
public:
  /**
   * Supported formats.
   */
  enum class Type {
    Serialize,
    APCSerialize,
  };

public:
  /**
   * Be aware that the default class_whitelist is null_array instead of
   * empty_array here because we do not limit the unserialization of arbitrary
   * class for hphp internal use
   */
  VariableUnserializer(const char *str, size_t len, Type type,
                       bool allowUnknownSerializableClass = false,
                       const Array& class_whitelist = null_array)
      : m_type(type), m_buf(str), m_end(str + len),
        m_unknownSerializable(allowUnknownSerializableClass),
        m_classWhiteList(class_whitelist) {}
  VariableUnserializer(const char *str, const char *end, Type type,
                       bool allowUnknownSerializableClass = false,
                       const Array& class_whitelist = null_array)
      : m_type(type), m_buf(str), m_end(end),
        m_unknownSerializable(allowUnknownSerializableClass),
        m_classWhiteList(class_whitelist) {}

  void set(const char *buf, const char *end);
  Type getType() const { return m_type;}
  bool allowUnknownSerializableClass() const { return m_unknownSerializable;}
  bool isWhitelistedClass(const String& cls_name) const;

  Variant unserialize();
  Variant unserializeKey();
  void add(Variant* v, Uns::Mode mode) {
    if (mode == Uns::Mode::Value) {
      m_refs.emplace_back(RefInfo(v));
    } else if (mode == Uns::Mode::Key) {
      // do nothing
    } else if (mode == Uns::Mode::ColValue) {
      m_refs.emplace_back(RefInfo::makeNonRefable(v));
    } else {
      assert(mode == Uns::Mode::ColKey);
      // We don't currently support using the 'r' encoding to refer
      // to collection keys, but eventually we'll need to make this
      // work to allow objects as keys. For now we encode collections
      // keys in m_refs using a null pointer.
      m_refs.emplace_back(RefInfo(nullptr));
    }
  }
  // getByVal() is used to resolve the 'r' encoding
  Variant* getByVal(int id) {
    if (id <= 0 || id > (int)m_refs.size()) return nullptr;
    Variant* ret = m_refs[id-1].var();
    if (!ret) {
      throw Exception("Referring to collection keys using the 'r' encoding "
                      "is not supported");
    }
    return ret;
  }
  // getByVal() is used to resolve the 'R' encoding
  Variant* getByRef(int id) {
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
  int64_t readInt();
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
  Variant &addVar();

 private:
  struct RefInfo {
    explicit RefInfo(Variant* v) : m_data(reinterpret_cast<uintptr_t>(v)) {}
    static RefInfo makeNonRefable(Variant* v) {
      RefInfo r(v);
      r.m_data |= 1;
      return r;
    }
    Variant* var() const {
      return reinterpret_cast<Variant*>(m_data & ~1);
    }
    bool canBeReferenced() const { return !(m_data & 1); }
  private:
    uintptr_t m_data;
  };

  Type m_type;
  const char *m_buf;
  const char *m_end;
  smart::vector<RefInfo> m_refs;
  smart::list<Variant> m_vars;
  bool m_unknownSerializable;
  const Array& m_classWhiteList;    // classes allowed to be unserialized

  void check() {
    if (m_buf >= m_end) {
      throw Exception("Unexpected end of buffer during unserialization");
    }
  }
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_VARIABLE_UNSERIALIZER_H_
