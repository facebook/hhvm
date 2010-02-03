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

IMPLEMENT_SMART_ALLOCATION(MapString, SmartAllocatorImpl::NeedRestore);
///////////////////////////////////////////////////////////////////////////////
// constructors

MapString::MapString(CVarRef k, CStrRef v) {
  insertKey(k);
  m_elems.push_back(v);
}

MapString::MapString(const std::vector<ArrayElement *> &elems,
                     bool replace /* = true */) {
  unsigned int size = elems.size();
  m_elems.reserve(size);
  for (unsigned int i = 0; i < size; i++) {
    ArrayElement *elem = elems[i];
    if (elem->hasName()) {
      uint idx = insertKey(elem->getName(), elem->getHash());
      if (idx < m_elems.size()) {
        if (replace) m_elems[idx] = elem->getString();
        continue;
      }
    } else {
      appendKey();
    }
    m_elems.push_back(elem->getString());
  }
}

MapString::MapString(const MapString *src) : Map(src) {
  m_elems = src->getElems();
}

MapString::MapString(const VectorString *src, CVarRef k, CStrRef v) {
  ASSERT(src);
  ASSERT(src->getIndex(k) < 0);
  appendImpl(m_elems, src->getElems(), k, v);
}

MapString::MapString(const VectorString *src, const MapString *elems,
                     ArrayOp op) {
  ASSERT(src && elems);
  appendImpl(m_elems, src->getElems(), elems, elems->getElems(), op);
}

MapString::MapString(const MapString *src, CStrRef v) {
  ASSERT(src);
  appendImpl(m_elems, src, src->getElems(), v);
}

MapString::MapString(const MapString *src, CVarRef k, CStrRef v) {
  ASSERT(src);
  appendImpl(m_elems, src, src->getElems(), k, v);
}

MapString::MapString(const MapString *src, const VectorString *elems,
                     ArrayOp op) {
  ASSERT(src && elems);
  appendImpl(m_elems, src, src->getElems(), elems->getElems(), op);
}

MapString::MapString(const MapString *src, const MapString *elems,
                     ArrayOp op) {
  ASSERT(src && elems);
  appendImpl(m_elems, src, src->getElems(), elems, elems->getElems(), op);
}

MapString::MapString(const VectorString *src, int eraseIndex) {
  ASSERT(src);
  appendImpl(m_elems, src->getElems(), eraseIndex);
}

MapString::MapString(const MapString *src, int eraseIndex) {
  ASSERT(src);
  appendImpl(m_elems, src, src->getElems(), eraseIndex);
}

///////////////////////////////////////////////////////////////////////////////
// implementing ArrayData

ssize_t MapString::size() const {
  return m_elems.size();
}

Variant MapString::getValue(ssize_t pos) const {
  ASSERT(pos >= 0 && pos < size());
  return m_elems[pos];
}

ArrayData *MapString::lval(Variant *&ret, bool copy) {
  MapVariant *escalated = NEW(MapVariant)(this);
  escalated->lval(ret, false);
  return escalated;
}

ArrayData *MapString::lval(int64 k, Variant *&ret, bool copy,
                           int64 prehash /* = -1 */) {
  MapVariant *escalated = NEW(MapVariant)(this);
  escalated->lval(k, ret, false, prehash);
  return escalated;
}

ArrayData *MapString::lval(litstr k, Variant *&ret, bool copy,
                           int64 prehash /* = -1 */) {
  MapVariant *escalated = NEW(MapVariant)(this);
  escalated->lval(k, ret, false, prehash);
  return escalated;
}

ArrayData *MapString::lval(CStrRef k, Variant *&ret, bool copy,
                           int64 prehash /* = -1 */) {
  MapVariant *escalated = NEW(MapVariant)(this);
  escalated->lval(k, ret, false, prehash);
  return escalated;
}

ArrayData *MapString::lval(CVarRef k, Variant *&ret, bool copy,
                           int64 prehash /* = -1 */) {
  MapVariant *escalated = NEW(MapVariant)(this);
  escalated->lval(k, ret, false, prehash);
  return escalated;
}

ArrayData *MapString::setImpl(CVarRef k, CVarRef v,
                              bool copy, int64 prehash /* = -1 */) {
  if (!v.isString()) {
    return NEW(MapVariant)(this, k, v);
  }
  if (copy) {
    return NEW(MapString)(this, k, v.toString());
  }
  uint index = insertKey(k, prehash);
  if (index < m_elems.size()) {
    m_elems[index] = v.toString();
  } else {
    m_elems.push_back(v.toString());
  }
  return NULL;
}

ArrayData *MapString::remove(int64 k, bool copy, int64 prehash /* = -1 */) {
  return remove(Variant(k), copy, prehash);
}

ArrayData *MapString::remove(litstr k, bool copy, int64 prehash /* = -1 */) {
  return remove(Variant(k), copy, prehash);
}

ArrayData *MapString::remove(CStrRef k, bool copy, int64 prehash /* = -1 */) {
  return remove(Variant(k), copy, prehash);
}

ArrayData *MapString::remove(CVarRef k, bool copy, int64 prehash /* = -1 */) {
  int index = getIndex(k, prehash);
  if (index >= 0) {
    if (size() == 1) {
      return StaticEmptyArray::Get();
    }
    if (copy) {
      return NEW(MapString)(this, index);
    }
    m_elems.remove(index);
    removeKey(k, index, prehash);
    if (index < m_pos) m_pos--;
  }
  return NULL;
}

ArrayData *MapString::copy() const {
  return NEW(MapString)(this);
}

ArrayData *MapString::append(CVarRef v, bool copy) {
  if (v.getType() != KindOfString) {
    return NEW(MapVariant)(this, v);
  }

  if (copy) {
    return NEW(MapString)(this, v.toString());
  }

  appendKey();
  m_elems.push_back(v.toString());
  return NULL;
}

ArrayData *MapString::append(const ArrayData *elems, ArrayOp op, bool copy) {
  ASSERT(elems);

  const EmptyArray *none = dynamic_cast<const EmptyArray *>(elems);
  if (none) {
    return NULL;
  }

  const VectorString *strs = dynamic_cast<const VectorString *>(elems);
  if (strs) {
    if (copy) {
      return NEW(MapString)(this, strs, op);
    }
    appendImpl(m_elems, strs->getElems(), op);
    return NULL;
  }

  const MapString *mapString = dynamic_cast<const MapString *>(elems);
  if (mapString) {
    if (copy) {
      return NEW(MapString)(this, mapString, op);
    }
    appendImpl(m_elems, mapString, mapString->getElems(), op);
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

ArrayData *MapString::insert(ssize_t pos, CVarRef v, bool copy) {
  if (pos >= size()) {
    return append(v, false);
  }
  if (pos < 0) pos = 0;

  if (!v.isString()) {
    ArrayData *ret = NEW(MapVariant)(this);
    ret->insert(pos, v, false);
    return ret;
  }

  if (copy) {
    ArrayData* ret = NEW(MapString)(this);
    ret->insert(pos, v, false);
    return ret;
  }

  insertKey(pos);
  m_elems.insert(pos, v.toString());
  return NULL;
}

void MapString::onSetStatic() {
  Map::onSetStatic();
  for (unsigned int i = 0; i < m_elems.size(); i++) {
    m_elems[i]->setStatic();
  }
}

///////////////////////////////////////////////////////////////////////////////
}
