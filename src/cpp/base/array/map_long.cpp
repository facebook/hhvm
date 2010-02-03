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

IMPLEMENT_SMART_ALLOCATION(MapLong, SmartAllocatorImpl::NeedRestore);
///////////////////////////////////////////////////////////////////////////////
// constructors

MapLong::MapLong(CVarRef k, int64 v) {
  insertKey(k);
  m_elems.push_back(v);
}

MapLong::MapLong(const std::vector<ArrayElement *> &elems,
                 bool replace /* = true */) {
  unsigned int size = elems.size();
  m_elems.reserve(size);
  for (unsigned int i = 0; i < size; i++) {
    ArrayElement *elem = elems[i];
    if (elem->hasName()) {
      uint idx = insertKey(elem->getName(), elem->getHash());
      if (idx < m_elems.size()) {
        if (replace) m_elems[idx] = elem->getInt64();
        continue;
      }
    } else {
      appendKey();
    }
    m_elems.push_back(elem->getInt64());
  }
}

MapLong::MapLong(const MapLong *src) : Map(src) {
  m_elems = src->getElems();
}

MapLong::MapLong(const VectorLong *src, CVarRef k, int64 v) {
  ASSERT(src);
  ASSERT(src->getIndex(k) < 0);
  appendImpl(m_elems, src->getElems(), k, v);
}

MapLong::MapLong(const VectorLong *src, const MapLong *elems, ArrayOp op) {
  ASSERT(src && elems);
  appendImpl(m_elems, src->getElems(), elems, elems->getElems(), op);
}

MapLong::MapLong(const MapLong *src, int64 v) {
  ASSERT(src);
  appendImpl(m_elems, src, src->getElems(), v);
}

MapLong::MapLong(const MapLong *src, CVarRef k, int64 v) {
  ASSERT(src);
  appendImpl(m_elems, src, src->getElems(), k, v);
}

MapLong::MapLong(const MapLong *src, const VectorLong *elems, ArrayOp op) {
  ASSERT(src && elems);
  appendImpl(m_elems, src, src->getElems(), elems->getElems(), op);
}

MapLong::MapLong(const MapLong *src, const MapLong *elems, ArrayOp op) {
  ASSERT(src && elems);
  appendImpl(m_elems, src, src->getElems(), elems, elems->getElems(), op);
}

MapLong::MapLong(const VectorLong *src, int eraseIndex) {
  ASSERT(src);
  appendImpl(m_elems, src->getElems(), eraseIndex);
}

MapLong::MapLong(const MapLong *src, int eraseIndex) {
  ASSERT(src);
  appendImpl(m_elems, src, src->getElems(), eraseIndex);
}

///////////////////////////////////////////////////////////////////////////////
// implementing ArrayData

ssize_t MapLong::size() const {
  return m_elems.size();
}

Variant MapLong::getValue(ssize_t pos) const {
  ASSERT(pos != ArrayData::invalid_index && pos < size());
  return m_elems[pos];
}

ArrayData *MapLong::lval(Variant *&ret, bool copy) {
  MapVariant *escalated = NEW(MapVariant)(this);
  escalated->lval(ret, false);
  return escalated;
}

ArrayData *MapLong::lval(int64   k, Variant *&ret, bool copy,
                         int64 prehash /* = -1 */) {
  MapVariant *escalated = NEW(MapVariant)(this);
  escalated->lval(k, ret, false, prehash);
  return escalated;
}
ArrayData *MapLong::lval(litstr  k, Variant *&ret, bool copy,
                         int64 prehash /* = -1 */) {
  MapVariant *escalated = NEW(MapVariant)(this);
  escalated->lval(k, ret, false, prehash);
  return escalated;
}
ArrayData *MapLong::lval(CStrRef k, Variant *&ret, bool copy,
                         int64 prehash /* = -1 */) {
  MapVariant *escalated = NEW(MapVariant)(this);
  escalated->lval(k, ret, false, prehash);
  return escalated;
}
ArrayData *MapLong::lval(CVarRef k, Variant *&ret, bool copy,
                         int64 prehash /* = -1 */) {
  MapVariant *escalated = NEW(MapVariant)(this);
  escalated->lval(k, ret, false, prehash);
  return escalated;
}

ArrayData *MapLong::setImpl(CVarRef k, CVarRef v,
                            bool copy, int64 prehash /* = -1 */) {
  if (!v.isInteger()) {
    return NEW(MapVariant)(this, k, v);
  }
  if (copy) {
    return NEW(MapLong)(this, k, v.toInt64());
  }
  uint index = insertKey(k, prehash);
  if (index < m_elems.size()) {
    m_elems[index] = v.toInt64();
  } else {
    m_elems.push_back(v.toInt64());
  }
  return NULL;
}

ArrayData *MapLong::remove(int64 k, bool copy, int64 prehash /* = -1 */) {
  return remove(Variant(k), copy, prehash);
}

ArrayData *MapLong::remove(litstr k, bool copy, int64 prehash /* = -1 */) {
  return remove(Variant(k), copy, prehash);
}

ArrayData *MapLong::remove(CStrRef k, bool copy, int64 prehash /* = -1 */) {
  return remove(Variant(k), copy, prehash);
}

ArrayData *MapLong::remove(CVarRef k, bool copy, int64 prehash /* = -1 */) {
  int index = getIndex(k, prehash);
  if (index >= 0) {
    if (size() == 1) {
      return StaticEmptyArray::Get();
    }
    if (copy) {
      return NEW(MapLong)(this, index);
    }
    m_elems.remove(index);
    removeKey(k, index, prehash);
    if (index < m_pos) m_pos--;
  }
  return NULL;
}

ArrayData *MapLong::copy() const {
  return NEW(MapLong)(this);
}

ArrayData *MapLong::append(CVarRef v, bool copy) {
  if (!v.isInteger()) {
    return NEW(MapVariant)(this, v);
  }

  if (copy) {
    return NEW(MapLong)(this, v.toInt64());
  }

  appendKey();
  m_elems.push_back(v.toInt64());
  return NULL;
}

ArrayData *MapLong::append(const ArrayData *elems, ArrayOp op, bool copy) {
  ASSERT(elems);

  const EmptyArray *none = dynamic_cast<const EmptyArray *>(elems);
  if (none) {
    return NULL;
  }

  const VectorLong *longs = dynamic_cast<const VectorLong *>(elems);
  if (longs) {
    if (copy) {
      return NEW(MapLong)(this, longs, op);
    }
    appendImpl(m_elems, longs->getElems(), op);
    return NULL;
  }

  const MapLong *mapLong = dynamic_cast<const MapLong *>(elems);
  if (mapLong) {
    if (copy) {
      return NEW(MapLong)(this, mapLong, op);
    }
    appendImpl(m_elems, mapLong, mapLong->getElems(), op);
    return NULL;
  }

  const SharedMap *mapShared = dynamic_cast<const SharedMap *>(elems);
  if (mapShared) {
    MapVariant *escalated = mapShared->escalateToMapVariant();
    MapVariant *ret = NEW(MapVariant)(this, escalated, op);
    DELETE(MapVariant)(escalated);
    return ret;
  }

  return NEW(MapVariant)(this, elems, op);
}

ArrayData *MapLong::insert(ssize_t pos, CVarRef v, bool copy) {
  if (pos >= size()) {
    return append(v, false);
  }
  if (pos == ArrayData::invalid_index) pos = 0;

  if (!v.isInteger()) {
    ArrayData *ret = NEW(MapVariant)(this);
    ret->insert(pos, v, false);
    return ret;
  }

  if (copy) {
    ArrayData* ret = NEW(MapLong)(this);
    ret->insert(pos, v, false);
    return ret;
  }

  insertKey(pos);
  m_elems.insert(pos, v.toInt64());
  return NULL;
}

///////////////////////////////////////////////////////////////////////////////
}
