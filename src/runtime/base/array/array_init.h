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
#ifndef __ARRAY_INIT_H__
#define __ARRAY_INIT_H__

#include <runtime/base/array/zend_array.h>
#include <runtime/base/array/vector_variant.h>
#include <runtime/base/array/map_variant.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// macros for creating vectors or maps

#define CREATE_VECTOR1(e) Array::Create(e)
#define CREATE_VECTOR2(e1, e2)                                          \
  Array(ArrayInit(2, true).set(0, e1).set(1, e2).create())
#define CREATE_VECTOR3(e1, e2, e3)                                      \
  Array(ArrayInit(3, true).set(0, e1).set(1, e2).set(2, e3).create())
#define CREATE_VECTOR4(e1, e2, e3, e4)                                  \
  Array(ArrayInit(4, true).set(0, e1).set(1, e2).set(2, e3).set(3, e4). \
                           create())
#define CREATE_VECTOR5(e1, e2, e3, e4, e5)                              \
  Array(ArrayInit(5, true).set(0, e1).set(1, e2).set(2, e3).set(3, e4). \
                           set(4, e5).create())
#define CREATE_VECTOR6(e1, e2, e3, e4, e5, e6)                          \
  Array(ArrayInit(6, true).set(0, e1).set(1, e2).set(2, e3).set(3, e4). \
                           set(4, e5).set(5, e6).create())

#define CREATE_MAP1(n, e) Array::Create(n, e)
#define CREATE_MAP2(n1, e1, n2, e2)                                       \
  Array(ArrayInit(2, false).set(0, n1, e1).set(1, n2, e2).create())
#define CREATE_MAP3(n1, e1, n2, e2, n3, e3)                               \
  Array(ArrayInit(3, false).set(0, n1, e1).set(1, n2, e2).set(2, n3, e3). \
                            create())
#define CREATE_MAP4(n1, e1, n2, e2, n3, e3, n4, e4)                       \
  Array(ArrayInit(4, false).set(0, n1, e1).set(1, n2, e2).set(2, n3, e3). \
                            set(3, n4, e4).create())
#define CREATE_MAP5(n1, e1, n2, e2, n3, e3, n4, e4, n5, e5)               \
  Array(ArrayInit(5, false).set(0, n1, e1).set(1, n2, e2).set(2, n3, e3). \
                            set(3, n4, e4).set(4, n5, e5).create())
#define CREATE_MAP6(n1, e1, n2, e2, n3, e3, n4, e4, n5, e5, n6, e6)       \
  Array(ArrayInit(6, false).set(0, n1, e1).set(1, n2, e2).set(2, n3, e3). \
                            set(3, n4, e4).set(4, n5, e5).set(5, n6, e6). \
                            create())

///////////////////////////////////////////////////////////////////////////////
// ArrayInit

/**
 * When an Array is created, ArrayInit completely skips the use of
 * ArrayElement, (the set methods mimic the constructors of ArrayElement).
 * The setRef method handles the case where the value needs to be a reference.
 */
class ArrayInit {
public:
  enum KindOf {
    KindOfVectorVariant,
    KindOfMapVariant,
    KindOfZendArray
  };

  ArrayInit(ssize_t n, bool isVector = false);
  ~ArrayInit() {
    ASSERT(m_data == NULL);
  }

  template<typename T>
  ArrayInit &set(int p, const T &value) {
    m_data->append(value, false);
    return *this;
  }

  ArrayInit &set(int p, CVarRef v) {
    m_data->append(v, false);
    return *this;
  }

  ArrayInit &setRef(int p, CVarRef v) {
    v.setContagious();
    Variant value = v;
    value.setContagious();
    m_data->append(value, false);
    return *this;
  }

  template<typename T>
  ArrayInit &set(int p, CVarRef name, const T &value, int64 prehash = -1,
                 bool keyConverted = false) {
    ASSERT(m_kind != KindOfVectorVariant);
    Variant v(value);
    m_data->set(keyConverted ? name : getName(name), v, false, prehash);
    return *this;
  }

  ArrayInit &set(int p, CVarRef name, CVarRef v, int64 prehash = -1,
                 bool keyConverted = false) {
    ASSERT(m_kind != KindOfVectorVariant);
    m_data->set(keyConverted ? name : getName(name), v, false, prehash);
    return *this;
  }

  ArrayInit &setRef(int p, CVarRef name, CVarRef v, int64 prehash = -1,
                    bool keyConverted = false) {
    ASSERT(m_kind != KindOfVectorVariant);
    v.setContagious();
    Variant value = v;
    value.setContagious();
    m_data->set(keyConverted ? name : getName(name), value, false, prehash);
    return *this;
  }

  ArrayData *create() {
    ArrayData *ret = m_data;
    m_data = NULL;
    return ret;
  }

  static Variant getName(CVarRef name) {
    if (name.isNull()) return "";
    switch (name.getType()) {
    case KindOfBoolean:
      return name.toBoolean() ? 1LL : 0LL;
    case KindOfByte:
    case KindOfInt16:
    case KindOfInt32:
    case KindOfInt64:
      return name.toInt64();
    case KindOfDouble:
    case LiteralString:
    case KindOfString:
      return name.toKey();
    default:
      break;
    }
    return name;
  }

private:
  ArrayData *m_data;
  KindOf m_kind;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif /* __ARRAY_INIT_H__ */
