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

#ifndef __HPHP_ARRAY_ELEMENT_H__
#define __HPHP_ARRAY_ELEMENT_H__

#include <cpp/base/type_string.h>
#include <cpp/base/type_variant.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * This class is purely for initializing an array.
 */
class ArrayElement {
 public:
  template<typename T>
    ArrayElement(const T &value) : m_value(value) {
  }
  ArrayElement(CVarRef v) {
    bool contagious = v.isContagious();
    m_value = v;
    if (contagious) {
      m_value.setContagious();
    }
  }

  template<typename T>
    ArrayElement(CVarRef name, const T &value, int64 prehash = -1)
    : m_name(name.isNull() ? "" : name), m_value(value), m_hash(prehash) {
  }
  ArrayElement(CVarRef name, CVarRef v, int64 prehash = -1)
    : m_hash(prehash) {
    bool contagious = v.isContagious();
    m_value = v;
    if (contagious) {
      m_value.setContagious();
    }
    // Defer assignment of name to after so that m_name doesn't steal
    // the ref if name and v are the same.
    m_name = name.isNull() ? "" : name;
  }

  bool hasName() const {
    return !m_name.isNull();
  }
  Variant getName() const {
    ASSERT(hasName());
    switch(m_name.getType()) {
    case KindOfBoolean:
      return m_name.toBoolean() ? 1LL : 0LL;
    case KindOfByte:
    case KindOfInt16:
    case KindOfInt32:
    case KindOfInt64:
      return m_name.toInt64();
    case KindOfDouble:
    case LiteralString:
    case KindOfString:
      return m_name.toKey();
    default:
      break;
    }
    return m_name;
  }

  int64 getHash() const {
    return m_hash;
  }

  DataType getType() const {
    if (!m_value.isContagious()) {
      switch (m_value.getType()) {
      case KindOfByte:
      case KindOfInt16:
      case KindOfInt32:
      case KindOfInt64:
        return KindOfInt64;
      case LiteralString:
      case KindOfString:
        return KindOfString;
      default:
        break;
      }
    }
    return KindOfVariant;
  }

  int64 getInt64() const {
    ASSERT(getType() == KindOfInt64);
    return m_value.toInt64();
  }
  String getString() const {
    ASSERT(getType() == KindOfString);
    return m_value.toString();
  }
  CVarRef getVariant() {
    return m_value;
  }

  /**
   * Memory allocator methods.
   */
  DECLARE_SMART_ALLOCATION_NOCALLBACKS(ArrayElement);
  void dump();

 private:
  Variant m_name;
  Variant m_value;
  int64 m_hash;
};

typedef std::vector<ArrayElement *> ArrayElementVec;

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_ARRAY_ELEMENT_H__
