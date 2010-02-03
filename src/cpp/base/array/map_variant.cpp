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

IMPLEMENT_SMART_ALLOCATION(MapVariant, SmartAllocatorImpl::NeedRestore);
///////////////////////////////////////////////////////////////////////////////
// constructors

MapVariant::MapVariant(CVarRef k, CVarRef v) {
  insertKey(k);
  m_elems.push_back(ArrayFuncs::element(v));
}

MapVariant::MapVariant(const std::vector<ArrayElement *> &elems,
                       bool replace /* = true */) {
  unsigned int size = elems.size();
  m_elems.reserve(size);
  for (unsigned int i = 0; i < size; i++) {
    ArrayElement *elem = elems[i];
    if (elem->hasName()) {
      uint idx = insertKey(elem->getName(), elem->getHash());
      if (idx < m_elems.size()) {
        if (replace) {
          Variant *&v = m_elems[idx];
          if (v) {
            ArrayFuncs::release(v);
          }
          v = ArrayFuncs::element(elem->getVariant());
        }
        continue;
      }
    } else {
      appendKey();
    }
    m_elems.push_back(ArrayFuncs::element(elem->getVariant()));
  }
}

MapVariant::MapVariant(const MapLong *src) : Map(src) {
  ASSERT(src);
  ArrayFuncs::append(m_elems, src->getElems());
}

MapVariant::MapVariant(const MapString *src) : Map(src) {
  ASSERT(src);
  ArrayFuncs::append(m_elems, src->getElems());
}

MapVariant::MapVariant(const MapVariant *src) : Map(src) {
  ASSERT(src);
  ArrayFuncs::append(m_elems, src->getElems());
}

MapVariant::MapVariant(const VectorLong *src) {
  ASSERT(src);
  appendImpl(m_elems, src->getElems(), Plus);
}
MapVariant::MapVariant(const VectorString *src) {
  ASSERT(src);
  appendImpl(m_elems, src->getElems(), Plus);
}
MapVariant::MapVariant(const VectorVariant *src) {
  ASSERT(src);
  appendImpl(m_elems, src->getElems(), Plus);
}

MapVariant::MapVariant(const VectorLong *src, CVarRef k, CVarRef v) {
  ASSERT(src);
  ASSERT(src->getIndex(k) < 0);
  appendImpl(m_elems, src->getElems(), k, ArrayFuncs::element(v));
}

MapVariant::MapVariant(const VectorString *src, CVarRef k, CVarRef v) {
  ASSERT(src);
  ASSERT(src->getIndex(k) < 0);
  appendImpl(m_elems, src->getElems(), k, ArrayFuncs::element(v));
}

MapVariant::MapVariant(const VectorVariant *src, CVarRef k, CVarRef v,
                       bool copy) {
  ASSERT(src);
  ASSERT(src->getIndex(k) < 0);

  const HphpVector<Variant *> &srcElems = src->getElems();
  m_nextIndex = srcElems.size();
  for (int i = 0; i < m_nextIndex; i++) {
    m_map[Variant((int64)i)] = i;
  }

  // Called by VectorVariant::setImpl(): special escalation that will NOT
  // make a copy of array elements if copy is not requested
  m_elems.reserve(m_nextIndex + 1);
  if (copy) {
    appendImpl(m_elems, src->getElems(), k, ArrayFuncs::element(v));
  } else {
    m_elems.append(srcElems);
    insertKey(k);
    m_elems.push_back(ArrayFuncs::element(v));
  }
}

MapVariant::MapVariant(const VectorLong *src, const MapString *elems,
                       ArrayOp op) {
  ASSERT(src && elems);
  appendImpl(m_elems, src->getElems(), elems, elems->getElems(), op);
}

MapVariant::MapVariant(const VectorString *src, const MapLong *elems,
                       ArrayOp op) {
  ASSERT(src && elems);
  appendImpl(m_elems, src->getElems(), elems, elems->getElems(), op);
}

MapVariant::MapVariant(const VectorVariant *src, const Map *elems,
                       ArrayOp op) {
  ASSERT(src && elems);

  const MapVariant *mapVariant = dynamic_cast<const MapVariant *>(elems);
  if (mapVariant) {
    appendImpl(m_elems, src->getElems(), elems, mapVariant->getElems(), op);
    return;
  }

  const MapLong *mapLong = dynamic_cast<const MapLong *>(elems);
  if (mapLong) {
    appendImpl(m_elems, src->getElems(), elems, mapLong->getElems(), op);
    return;
  }

  const MapString *mapString = dynamic_cast<const MapString *>(elems);
  if (mapString) {
    appendImpl(m_elems, src->getElems(), elems, mapString->getElems(), op);
    return;
  }

  ASSERT(false);
}

MapVariant::MapVariant(const MapLong *src, CVarRef v) {
  ASSERT(src);
  appendImpl(m_elems, src, src->getElems(), ArrayFuncs::element(v));
}

MapVariant::MapVariant(const MapString *src, CVarRef v) {
  ASSERT(src);
  appendImpl(m_elems, src, src->getElems(), ArrayFuncs::element(v));
}

MapVariant::MapVariant(const MapVariant *src, CVarRef v) {
  ASSERT(src);
  appendImpl(m_elems, src, src->getElems(), ArrayFuncs::element(v));
}

MapVariant::MapVariant(const MapLong *src, CVarRef k, CVarRef v) {
  ASSERT(src);
  appendImpl(m_elems, src, src->getElems(), k, ArrayFuncs::element(v));
}

MapVariant::MapVariant(const MapString *src, CVarRef k, CVarRef v) {
  ASSERT(src);
  appendImpl(m_elems, src, src->getElems(), k, ArrayFuncs::element(v));
}

MapVariant::MapVariant(const MapVariant *src, CVarRef k, CVarRef v) {
  ASSERT(src);
  appendImpl(m_elems, src, src->getElems(), k, ArrayFuncs::element(v));
}

MapVariant::MapVariant(const VectorLong *src, const MapVariant *elems,
                       ArrayOp op) {
  ASSERT(src && elems);
  appendImpl(m_elems, src->getElems(), elems, elems->getElems(), op);
}

MapVariant::MapVariant(const VectorString *src, const MapVariant *elems,
                       ArrayOp op) {
  ASSERT(src && elems);
  appendImpl(m_elems, src->getElems(), elems, elems->getElems(), op);
}

MapVariant::MapVariant(const MapLong *src, const ArrayData *elems,
                       ArrayOp op) {
  ASSERT(src && elems);

  const MapVariant *mapVariant = dynamic_cast<const MapVariant *>(elems);
  if (mapVariant) {
    appendImpl(m_elems, src, src->getElems(), mapVariant,
               mapVariant->getElems(), op);
    return;
  }

  const VectorString *vecString = dynamic_cast<const VectorString *>(elems);
  if (vecString) {
    appendImpl(m_elems, src, src->getElems(), vecString->getElems(), op);
    return;
  }

  const VectorVariant *vecVariant = dynamic_cast<const VectorVariant *>(elems);
  if (vecVariant) {
    appendImpl(m_elems, src, src->getElems(), vecVariant->getElems(), op);
    return;
  }

  const MapLong *mapLong = dynamic_cast<const MapLong *>(elems);
  if (mapLong) {
    appendImpl(m_elems, src, src->getElems(), mapLong, mapLong->getElems(),
               op);
    return;
  }

  const MapString *mapString = dynamic_cast<const MapString *>(elems);
  if (mapString) {
    appendImpl(m_elems, src, src->getElems(), mapString, mapString->getElems(),
               op);
    return;
  }

  const VectorLong *vecLong = dynamic_cast<const VectorLong *>(elems);
  if (vecLong) {
    appendImpl(m_elems, src, src->getElems(), vecLong->getElems(), op);
    return;
  }

  ASSERT(false);
}

MapVariant::MapVariant(const MapString *src, const ArrayData *elems,
                       ArrayOp op) {
  ASSERT(src && elems);

  const MapVariant *mapVariant = dynamic_cast<const MapVariant *>(elems);
  if (mapVariant) {
    appendImpl(m_elems, src, src->getElems(), mapVariant,
               mapVariant->getElems(), op);
    return;
  }

  const VectorString *vecString = dynamic_cast<const VectorString *>(elems);
  if (vecString) {
    appendImpl(m_elems, src, src->getElems(), vecString->getElems(), op);
    return;
  }

  const VectorVariant *vecVariant = dynamic_cast<const VectorVariant *>(elems);
  if (vecVariant) {
    appendImpl(m_elems, src, src->getElems(), vecVariant->getElems(), op);
    return;
  }

  const MapLong *mapLong = dynamic_cast<const MapLong *>(elems);
  if (mapLong) {
    appendImpl(m_elems, src, src->getElems(), mapLong, mapLong->getElems(),
               op);
    return;
  }

  const MapString *mapString = dynamic_cast<const MapString *>(elems);
  if (mapString) {
    appendImpl(m_elems, src, src->getElems(), mapString,
               mapString->getElems(), op);
    return;
  }

  const VectorLong *vecLong = dynamic_cast<const VectorLong *>(elems);
  if (vecLong) {
    appendImpl(m_elems, src, src->getElems(), vecLong->getElems(), op);
    return;
  }

  ASSERT(false);
}

MapVariant::MapVariant(const MapVariant *src, const ArrayData *elems,
                       ArrayOp op) {
  ASSERT(src && elems);

  const MapVariant *mapVariant = dynamic_cast<const MapVariant *>(elems);
  if (mapVariant) {
    appendImpl(m_elems, src, src->getElems(), mapVariant,
               mapVariant->getElems(), op);
    return;
  }

  const VectorString *vecString = dynamic_cast<const VectorString *>(elems);
  if (vecString) {
    appendImpl(m_elems, src, src->getElems(), vecString->getElems(), op);
    return;
  }

  const VectorVariant *vecVariant = dynamic_cast<const VectorVariant *>(elems);
  if (vecVariant) {
    appendImpl(m_elems, src, src->getElems(), vecVariant->getElems(), op);
    return;
  }

  const MapLong *mapLong = dynamic_cast<const MapLong *>(elems);
  if (mapLong) {
    appendImpl(m_elems, src, src->getElems(), mapLong, mapLong->getElems(),
               op);
    return;
  }

  const MapString *mapString = dynamic_cast<const MapString *>(elems);
  if (mapString) {
    appendImpl(m_elems, src, src->getElems(), mapString,
               mapString->getElems(), op);
    return;
  }

  const VectorLong *vecLong = dynamic_cast<const VectorLong *>(elems);
  if (vecLong) {
    appendImpl(m_elems, src, src->getElems(), vecLong->getElems(), op);
    return;
  }

  ASSERT(false);
}

MapVariant::MapVariant(const VectorVariant *src, int eraseIndex) {
  ASSERT(src);

  int iMax = src->getElems().size();
  if (iMax > 0) {
    HphpVector<Variant*> elems;
    elems.reserve(iMax);
    for (int i = 0; i < iMax; i++) {
      elems.push_back(src->getElems()[i]);
    }
    appendImpl(m_elems, elems, eraseIndex);
  }
}

MapVariant::MapVariant(const MapVariant *src, int eraseIndex) {
  ASSERT(src);
  appendImpl(m_elems, src, src->getElems(), eraseIndex);
}

MapVariant::~MapVariant() {
  for (int i = m_elems.size() - 1; i >= 0; --i) {
    ArrayFuncs::release(m_elems[i]);
  }
}

///////////////////////////////////////////////////////////////////////////////
// implementing ArrayData

ssize_t MapVariant::size() const {
  return m_elems.size();
}

Variant MapVariant::getValue(ssize_t pos) const {
  ASSERT(pos >= 0 && pos < size());
  return *m_elems[pos];
}

CVarRef MapVariant::getValueRef(ssize_t pos) const {
  ASSERT(pos >= 0 && pos < size());
  return *m_elems[pos];
}

ArrayData *MapVariant::lval(Variant *&ret, bool copy) {
  ASSERT(!m_elems.empty());

  if (copy) {
    MapVariant* data = NEW(MapVariant)(this);
    ret = data->m_elems.back();
    return data;
  }

  ret = m_elems.back();
  return NULL;
}

ArrayData *MapVariant::lval(int64 k, Variant *&ret, bool copy,
                            int64 prehash /* = -1 */) {
  return lval(Variant(k), ret, copy, prehash);
}

ArrayData *MapVariant::lval(litstr k, Variant *&ret, bool copy,
                            int64 prehash /* = -1 */) {
  return lval(Variant(k), ret, copy, prehash);
}

ArrayData *MapVariant::lval(CStrRef k, Variant *&ret, bool copy,
                            int64 prehash /* = -1 */) {
  return lval(Variant(k), ret, copy, prehash);
}

ArrayData *MapVariant::lval(CVarRef k, Variant *&ret, bool copy,
                            int64 prehash /* = -1 */) {
  if (copy) {
    MapVariant* data = NEW(MapVariant)(this);
    ArrayData* data2 = data->lval(k, ret, false, prehash);
    if (data2) {
      data->release();
      return data2;
    }
    return data;
  }

  ssize_t index = insertKey(k, prehash);
  if (index >= m_elems.size()) {
    ret = NEW(Variant)();
    m_elems.push_back(ret);
  } else {
    ret = m_elems[index];
  }
  return NULL;
}

ArrayData *MapVariant::setImpl(CVarRef k, CVarRef v,
                               bool copy, int64 prehash /* = -1 */) {
  if (copy) {
    return NEW(MapVariant)(this, k, v);
  }
  uint index = insertKey(k, prehash);
  if (index < m_elems.size()) {
    *m_elems[index] = v;
  } else {
    m_elems.push_back(ArrayFuncs::element(v));
  }
  return NULL;
}

ArrayData *MapVariant::remove(int64 k, bool copy, int64 prehash /* = -1 */) {
  return remove(Variant(k), copy, prehash);
}

ArrayData *MapVariant::remove(litstr k, bool copy, int64 prehash /* = -1 */) {
  return remove(Variant(k), copy, prehash);
}

ArrayData *MapVariant::remove(CStrRef k, bool copy, int64 prehash /* = -1 */) {
  return remove(Variant(k), copy, prehash);
}

ArrayData *MapVariant::remove(CVarRef k, bool copy, int64 prehash /* = -1 */) {
  int index = getIndex(k, prehash);
  if (index >= 0) {
    if (size() == 1) {
      return StaticEmptyArray::Get();
    }
    if (copy) {
      return NEW(MapVariant)(this, index);
    }
    m_elems[index]->release();
    m_elems.remove(index);
    removeKey(k, index, prehash);
    if (index < m_pos) m_pos--;
  }
  return NULL;
}

ArrayData *MapVariant::copy() const {
  return NEW(MapVariant)(this);
}

ArrayData *MapVariant::append(CVarRef v, bool copy) {
  if (copy) {
    return NEW(MapVariant)(this, v);
  }

  appendKey();
  m_elems.push_back(ArrayFuncs::element(v));
  return NULL;
}

ArrayData *MapVariant::append(const ArrayData *elems, ArrayOp op, bool copy) {
  ASSERT(elems);

  const EmptyArray *none = dynamic_cast<const EmptyArray *>(elems);
  if (none) {
    return NULL;
  }

  const SharedMap *mapShared = dynamic_cast<const SharedMap *>(elems);
  if (mapShared) {
    MapVariant *escalated = mapShared->escalateToMapVariant();
    MapVariant *ret = NULL;
    if (copy) {
      ret = NEW(MapVariant)(this, escalated, op);
    } else {
      appendImpl(m_elems, escalated, escalated->getElems(), op);
    }
    DELETE(MapVariant)(escalated);
    return ret;
  }

  if (copy) {
    return NEW(MapVariant)(this, elems, op);
  }

  const MapVariant *mapVariant = dynamic_cast<const MapVariant *>(elems);
  if (mapVariant) {
    appendImpl(m_elems, mapVariant, mapVariant->getElems(), op);
    return NULL;
  }

  const VectorString *vecString = dynamic_cast<const VectorString *>(elems);
  if (vecString) {
    appendImpl(m_elems, vecString->getElems(), op);
    return NULL;
  }

  const VectorVariant *vecVariant = dynamic_cast<const VectorVariant *>(elems);
  if (vecVariant) {
    appendImpl(m_elems, vecVariant->getElems(), op);
    return NULL;
  }

  const MapLong *mapLong = dynamic_cast<const MapLong *>(elems);
  if (mapLong) {
    appendImpl(m_elems, mapLong, mapLong->getElems(), op);
    return NULL;
  }

  const MapString *mapString = dynamic_cast<const MapString *>(elems);
  if (mapString) {
    appendImpl(m_elems, mapString, mapString->getElems(), op);
    return NULL;
  }

  const VectorLong *vecLong = dynamic_cast<const VectorLong *>(elems);
  if (vecLong) {
    appendImpl(m_elems, vecLong->getElems(), op);
    return NULL;
  }

  ASSERT(false);
  return NULL;
}

ArrayData *MapVariant::insert(ssize_t pos, CVarRef v, bool copy) {
  if (pos >= size()) {
    return append(v, false);
  }
  if (pos < 0) pos = 0;

  if (copy) {
    ArrayData* ret = NEW(MapVariant)(this);
    ret->insert(pos, v, false);
    return ret;
  }

  insertKey(pos);
  m_elems.insert(pos, ArrayFuncs::element(v));
  return NULL;
}

void MapVariant::onSetStatic() {
  Map::onSetStatic();
  for (unsigned int i = 0; i < m_elems.size(); i++) {
    m_elems[i]->setStatic();
  }
}

void MapVariant::getFullPos(FullPos &pos) {
  if (m_pos != ArrayData::invalid_index) {
    Variant k = getKey(m_pos);
    const HphpMapCell &c = m_map.findCell(k);
    pos.primary = c.hash();
    pos.secondary = c.num();
  } else {
    pos.primary = ArrayData::invalid_index;
  }
}
bool MapVariant::setFullPos(const FullPos &pos) {
  // Only set if pos hasn't been invalidated.
  if (pos.primary != ArrayData::invalid_index) {
    m_pos = m_map.rawFind(pos.primary, pos.secondary);
  }
  if (m_pos == ArrayData::invalid_index || m_pos >= size()) return false;
  return true;
}

///////////////////////////////////////////////////////////////////////////////
}
