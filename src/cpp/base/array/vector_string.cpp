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

#include <cpp/base/array/empty_array.h>
#include <cpp/base/array/vector_long.h>
#include <cpp/base/array/vector_string.h>
#include <cpp/base/array/vector_variant.h>
#include <cpp/base/array/map_long.h>
#include <cpp/base/array/map_string.h>
#include <cpp/base/array/map_variant.h>
#include <cpp/base/shared/shared_map.h>
#include <cpp/base/array/array_element.h>

namespace HPHP {

IMPLEMENT_SMART_ALLOCATION(VectorString, SmartAllocatorImpl::NeedRestore);
///////////////////////////////////////////////////////////////////////////////
// constructors

VectorString::VectorString(CStrRef v) {
  m_elems.push_back(v);
}

VectorString::VectorString(const std::vector<ArrayElement *> &elems) {
  unsigned int size = elems.size();
  m_elems.reserve(size);
  for (unsigned int i = 0; i < size; i++) {
    m_elems.push_back(elems[i]->getString());
  }
}

VectorString::VectorString(const VectorString *src) : Vector(src) {
  ASSERT(src);
  m_elems = src->getElems();
}

VectorString::VectorString(const VectorString *src, CStrRef v) {
  ASSERT(src);
  appendImpl(m_elems, src->getElems(), v);
}

VectorString::VectorString(const VectorString *src, int index, CStrRef v) {
  ASSERT(src);
  appendImpl(m_elems, src->getElems(), index, v);
}

VectorString::VectorString(const VectorString *src, const VectorString *elems,
                           ArrayOp op) {
  ASSERT(src && elems);
  appendImpl(m_elems, src->getElems(), elems->getElems(), op);
}

///////////////////////////////////////////////////////////////////////////////
// implementing ArrayData

ssize_t VectorString::size() const {
  return m_elems.size();
}

Variant VectorString::getValue(ssize_t pos) const {
  ASSERT(pos >= 0 && pos < size());
  return m_elems[pos];
}

ArrayData *VectorString::lval(Variant *&ret, bool copy) {
  VectorVariant *escalated = NEW(VectorVariant)(this);
  escalated->lval(ret, false);
  return escalated;
}

ArrayData *VectorString::lval(int64 k, Variant *&ret, bool copy,
                              int64 prehash /* = -1 */) {
  VectorVariant *escalated = NEW(VectorVariant)(this);
  escalated->lval(k, ret, false, prehash);
  return escalated;
}

ArrayData *VectorString::lval(litstr k, Variant *&ret, bool copy,
                              int64 prehash /* = -1 */) {
  MapVariant *escalated = NEW(MapVariant)(this);
  escalated->lval(k, ret, false, prehash);
  return escalated;
}

ArrayData *VectorString::lval(CStrRef k, Variant *&ret, bool copy,
                              int64 prehash /* = -1 */) {
  MapVariant *escalated = NEW(MapVariant)(this);
  escalated->lval(k, ret, false, prehash);
  return escalated;
}

ArrayData *VectorString::lval(CVarRef k, Variant *&ret, bool copy,
                              int64 prehash /* = -1 */) {
  ArrayData *escalated;
  if (k.isInteger()) {
    escalated = NEW(VectorVariant)(this);
  } else {
    escalated = NEW(MapVariant)(this);
  }
  escalated->lval(k, ret, false, prehash);
  return escalated;
}

ArrayData *VectorString::set(int64 k, CVarRef v,
                             bool copy, int64 prehash /* = -1 */) {
  if (k == (int64)m_elems.size()) {
    return append(v, copy);
  }
  return setImpl(k, v, copy);
}

ArrayData *VectorString::set(litstr k, CVarRef v,
                             bool copy, int64 prehash /* = -1 */) {
  return setImpl(k, v, copy);
}

ArrayData *VectorString::set(CStrRef k, CVarRef v,
                             bool copy, int64 prehash /* = -1 */) {
  return setImpl(k, v, copy);
}

ArrayData *VectorString::set(CVarRef k, CVarRef v,
                             bool copy, int64 prehash /* = -1 */) {
  if (k.isInteger()) {
    return set(k.toInt64(), v, copy);
  }
  return setImpl(k.toString(), v, copy);
}

ArrayData *VectorString::remove(int64 k, bool copy, int64 prehash /* = -1 */) {
  return removeImpl(getIndex(k), copy);
}

ArrayData *VectorString::remove(litstr k, bool copy, int64 prehash /* = -1 */) {
  return removeImpl(getIndex(k), copy);
}

ArrayData *VectorString::remove(CStrRef k, bool copy,
                                int64 prehash /* = -1 */) {
  return removeImpl(getIndex(k), copy);
}

ArrayData *VectorString::remove(CVarRef k, bool copy,
                                int64 prehash /* = -1 */) {
  return removeImpl(getIndex(k), copy);
}

ArrayData *VectorString::copy() const {
  return NEW(VectorString)(this);
}

ArrayData *VectorString::append(CVarRef v, bool copy) {
  if (!v.isString()) {
    return NEW(VectorVariant)(this, v);
  }

  if (copy) {
    return NEW(VectorString)(this, v.toString());
  }
  m_elems.push_back(v.toString());
  return NULL;
}

ArrayData *VectorString::append(const ArrayData *elems, ArrayOp op,
                                bool copy) {
  ASSERT(elems);

  const EmptyArray *none = dynamic_cast<const EmptyArray *>(elems);
  if (none) {
    return NULL;
  }

  const VectorString *strs = dynamic_cast<const VectorString *>(elems);
  if (strs) {
    if (copy) {
      return NEW(VectorString)(this, strs, op);
    }
    appendImpl(m_elems, strs->getElems(), op);
    return NULL;
  }

  // Even though resulting array may still be representable as VectorString,
  // the fact that we are appending/merging heterogenous data implies this
  // array is variant by nature.
  const Vector *vec = dynamic_cast<const Vector *>(elems);
  if (vec) {
    return NEW(VectorVariant)(this, vec, op);
  }
  const MapVariant *mapVariant = dynamic_cast<const MapVariant *>(elems);
  if (mapVariant) {
    return NEW(MapVariant)(this, mapVariant, op);
  }
  const MapString *mapString = dynamic_cast<const MapString *>(elems);
  if (mapString) {
    return NEW(MapString)(this, mapString, op);
  }
  const MapLong *mapLong = dynamic_cast<const MapLong *>(elems);
  if (mapLong) {
    return NEW(MapVariant)(this, mapLong, op);
  }
  const SharedMap *mapShared = dynamic_cast<const SharedMap *>(elems);
  if (mapShared) {
    MapVariant *escalated = mapShared->escalateToMapVariant();
    MapVariant *ret = NEW(MapVariant)(this, escalated, op);
    DELETE(MapVariant)(escalated);
    return ret;
  }
  ASSERT(false);
  return NULL;
}

ArrayData *VectorString::insert(ssize_t pos, CVarRef v, bool copy) {
  if (!v.isString()) {
    ArrayData *ret = NEW(VectorVariant)(this);
    ret->insert(pos, v, false);
    return ret;
  }

  if (copy) {
    ArrayData* ret = NEW(VectorString)(this);
    ret->insert(pos, v, false);
    return ret;
  }

  if (pos >= size()) {
    return append(v, false);
  }
  if (pos < 0) pos = 0;

  m_elems.insert(pos, v.toString());
  return NULL;
}

void VectorString::onSetStatic() {
  for (unsigned int i = 0; i < m_elems.size(); i++) {
    m_elems[i]->setStatic();
  }
}

///////////////////////////////////////////////////////////////////////////////
// helpers

Variant VectorString::getImpl(int index) const {
  if (index >= 0) {
    return m_elems[index];
  }
  return null;
}

bool VectorString::setImpl(int index, CVarRef v, bool copy, ArrayData *&ret) {
  bool keepVector = false;
  if (index >= 0) {
    keepVector = true;
  }

  if (!v.isString()) {
    if (keepVector) {
      ret = NEW(VectorVariant)(this, index, v);
      return true;
    }
    return false;
  }

  if (!keepVector) {
    return false;
  }
  if (copy) {
    ret = NEW(VectorString)(this, index, v.toString());
    return true;
  }

  if ((ssize_t)index < size()) {
    ASSERT(index >= 0);
    m_elems[index] = v.toString();
  } else {
    ASSERT((ssize_t)index == size());
    m_elems.push_back(v.toString());
  }
  ret = NULL;
  return true;
}

ArrayData *VectorString::removeImpl(int index, bool copy) {
  if (index >= 0) {
    if (size() == 1) {
      return StaticEmptyArray::Get();
    }
    if (copy || (ssize_t)index < size() - 1) {
      return NEW(MapString)(this, index);
    }
    m_elems.remove(index);
  }
  return NULL;
}

///////////////////////////////////////////////////////////////////////////////
}
