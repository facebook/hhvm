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

IMPLEMENT_SMART_ALLOCATION(VectorLong, SmartAllocatorImpl::NeedRestore);
///////////////////////////////////////////////////////////////////////////////
// constructors

VectorLong::VectorLong(int64 v) {
  m_elems.push_back(v);
}

VectorLong::VectorLong(const std::vector<ArrayElement *> &elems) {
  unsigned int size = elems.size();
  m_elems.reserve(size);
  for (unsigned int i = 0; i < size; i++) {
    m_elems.push_back(elems[i]->getInt64());
  }
}

VectorLong::VectorLong(const VectorLong *src) : Vector(src) {
  ASSERT(src);
  m_elems = src->getElems();
}

VectorLong::VectorLong(const VectorLong *src, int64 v) {
  ASSERT(src);
  appendImpl(m_elems, src->getElems(), v);
}

VectorLong::VectorLong(const VectorLong *src, int index, int64 v) {
  ASSERT(src);
  appendImpl(m_elems, src->getElems(), index, v);
}

VectorLong::VectorLong(const VectorLong *src, const VectorLong *elems,
                       ArrayOp op) {
  ASSERT(src && elems);
  appendImpl(m_elems, src->getElems(), elems->getElems(), op);
}

///////////////////////////////////////////////////////////////////////////////
// implementing ArrayData

ssize_t VectorLong::size() const {
  return m_elems.size();
}

Variant VectorLong::getValue(ssize_t pos) const {
  ASSERT(pos >= 0 && pos < size());
  return m_elems[pos];
}

ArrayData *VectorLong::lval(Variant *&ret, bool copy) {
  VectorVariant *escalated = NEW(VectorVariant)(this);
  escalated->lval(ret, false);
  return escalated;
}

ArrayData *VectorLong::lval(int64 k, Variant *&ret, bool copy,
                            int64 prehash /* = -1 */) {
  VectorVariant *escalated = NEW(VectorVariant)(this);
  escalated->lval(k, ret, false, prehash);
  return escalated;
}

ArrayData *VectorLong::lval(litstr k, Variant *&ret, bool copy,
                            int64 prehash /* = -1 */) {
  MapVariant *escalated = NEW(MapVariant)(this);
  escalated->lval(k, ret, false, prehash);
  return escalated;
}

ArrayData *VectorLong::lval(CStrRef k, Variant *&ret, bool copy,
                            int64 prehash /* = -1 */) {
  MapVariant *escalated = NEW(MapVariant)(this);
  escalated->lval(k, ret, false, prehash);
  return escalated;
}

ArrayData *VectorLong::lval(CVarRef k, Variant *&ret, bool copy,
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

ArrayData *VectorLong::set(int64 k, CVarRef v,
                           bool copy, int64 prehash /* = -1 */) {
  if (k == (int64)m_elems.size()) {
    return append(v, copy);
  }
  return setImpl(k, v, copy);
}

ArrayData *VectorLong::set(litstr k, CVarRef v,
                           bool copy, int64 prehash /* = -1 */) {
  return setImpl(k, v, copy);
}

ArrayData *VectorLong::set(CStrRef k, CVarRef v,
                           bool copy, int64 prehash /* = -1 */) {
  return setImpl(k, v, copy);
}

ArrayData *VectorLong::set(CVarRef k, CVarRef v,
                           bool copy, int64 prehash /* = -1 */) {
  if (k.isInteger()) {
    return set(k.toInt64(), v, copy);
  }
  return setImpl(k.toString(), v, copy);
}

ArrayData *VectorLong::remove(int64 k, bool copy, int64 prehash /* = -1 */) {
  return removeImpl(getIndex(k), copy);
}

ArrayData *VectorLong::remove(litstr k, bool copy, int64 prehash /* = -1 */) {
  return removeImpl(getIndex(k), copy);
}

ArrayData *VectorLong::remove(CStrRef k, bool copy, int64 prehash /* = -1 */) {
  return removeImpl(getIndex(k), copy);
}

ArrayData *VectorLong::remove(CVarRef k, bool copy, int64 prehash /* = -1 */) {
  return removeImpl(getIndex(k), copy);
}

ArrayData *VectorLong::copy() const {
  return NEW(VectorLong)(this);
}

ArrayData *VectorLong::append(CVarRef v, bool copy) {
  if (!v.isInteger()) {
    return NEW(VectorVariant)(this, v);
  }

  if (copy) {
    return NEW(VectorLong)(this, v.toInt64());
  }
  m_elems.push_back(v.toInt64());
  return NULL;
}

ArrayData *VectorLong::append(const ArrayData *elems, ArrayOp op, bool copy) {
  ASSERT(elems);

  const EmptyArray *none = dynamic_cast<const EmptyArray *>(elems);
  if (none) {
    return NULL;
  }

  const VectorLong *longs = dynamic_cast<const VectorLong *>(elems);
  if (longs) {
    if (copy) {
      return NEW(VectorLong)(this, longs, op);
    }
    appendImpl(m_elems, longs->getElems(), op);
    return NULL;
  }

  // Even though resulting array may still be representable as VectorLong,
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
  const MapLong *mapLong = dynamic_cast<const MapLong *>(elems);
  if (mapLong) {
    return NEW(MapLong)(this, mapLong, op);
  }
  const MapString *mapString = dynamic_cast<const MapString *>(elems);
  if (mapString) {
    return NEW(MapVariant)(this, mapString, op);
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

ArrayData *VectorLong::insert(ssize_t pos, CVarRef v, bool copy) {
  if (!v.isInteger()) {
    ArrayData *ret = NEW(VectorVariant)(this);
    ret->insert(pos, v, false);
    return ret;
  }

  if (copy) {
    ArrayData* ret = NEW(VectorLong)(this);
    ret->insert(pos, v, false);
    return ret;
  }

  if (pos >= size()) {
    return append(v, false);
  }
  if (pos < 0) pos = 0;

  m_elems.insert(pos, v.toInt64());
  return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// helpers

Variant VectorLong::getImpl(int index) const {
  if (index >= 0) {
    return m_elems[index];
  }
  return null;
}

bool VectorLong::setImpl(int index, CVarRef v, bool copy, ArrayData *&ret) {
  bool keepVector = false;
  if (index >= 0) {
    keepVector = true;
  }

  if (!v.isInteger()) {
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
    ret = NEW(VectorLong)(this, index, v.toInt64());
    return true;
  }

  if ((ssize_t)index < size()) {
    ASSERT(index >= 0);
    m_elems[index] = v.toInt64();
  } else {
    ASSERT((ssize_t)index == size());
    m_elems.push_back(v.toInt64());
  }
  ret = NULL;
  return true;
}

ArrayData *VectorLong::removeImpl(int index, bool copy) {
  if (index >= 0) {
    if (size() == 1) {
      return StaticEmptyArray::Get();
    }
    if (copy || (ssize_t)index < size() - 1) {
      return NEW(MapLong)(this, index);
    }
    m_elems.remove(index);
  }
  return NULL;
}

///////////////////////////////////////////////////////////////////////////////
}
