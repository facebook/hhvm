/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_STRUCT_ARRAY_DEFS_H_
#define incl_HPHP_STRUCT_ARRAY_DEFS_H_

#include "hphp/runtime/base/shape.h"

namespace HPHP {

inline StructArray* StructArray::asStructArray(ArrayData* ad) {
  assert(ad->kind() == kStructKind);
  return static_cast<StructArray*>(ad);
}

inline const StructArray* StructArray::asStructArray(const ArrayData* ad) {
  assert(ad->kind() == kStructKind);
  return static_cast<const StructArray*>(ad);
}

inline Shape* StructArray::shape() {
  return m_shape;
}

inline Shape* StructArray::shape() const {
  return m_shape;
}

inline void StructArray::setShape(Shape* shape) {
  m_shape = shape;
  m_size = shape->size();
}

inline size_t StructArray::size() const {
  return ArrayData::size();
}

inline size_t StructArray::capacity() const {
  return shape()->capacity();
}

inline TypedValue* StructArray::data() {
  return reinterpret_cast<TypedValue*>(this + 1);
}

inline const TypedValue* StructArray::data() const {
  return reinterpret_cast<const TypedValue*>(this + 1);
}

inline size_t StructArray::heapSize(const ArrayData* ad) {
  return bytesForCapacity(asStructArray(ad)->capacity());
}

template<class F> void StructArray::scan(F& mark) const {
  //mark(m_shape); not in php heap
  for (unsigned i = 0, n = size(); i < n; ++i) {
    mark(data()[i]);
  }
}
}

#endif
