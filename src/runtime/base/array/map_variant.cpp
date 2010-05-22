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

#include <runtime/base/array/map_variant.h>
#include <runtime/base/shared/shared_map.h>
#include <runtime/base/types.h>
#include <vector>

namespace HPHP {

IMPLEMENT_SMART_ALLOCATION(MapVariant, SmartAllocatorImpl::NeedRestore);
///////////////////////////////////////////////////////////////////////////////
// constructors

MapVariant::MapVariant(CVarRef k, CVarRef v) :
    m_nextIndex(0), m_keys(NULL) {
  insertKey(k);
  m_elems.push_back(ArrayFuncs::element(v));
}

MapVariant::MapVariant(const MapVariant *src) :
    ArrayData(src), m_nextIndex(0), m_keys(NULL) {
  ASSERT(src);
  m_map = src->m_map;
  m_nextIndex = src->m_nextIndex;
  ArrayFuncs::append(m_elems, src->getElems());
}

MapVariant::MapVariant(const VectorVariant *src) :
    m_nextIndex(0), m_keys(NULL) {
  ASSERT(src);
  const HphpVector<Variant *> &srcElems = src->getElems();
  m_nextIndex = srcElems.size();
  for (int i = 0; i < m_nextIndex; i++) {
    m_map[Variant((int64)i)] = i;
  }
  m_elems.reserve(m_nextIndex);
  ArrayFuncs::append(m_elems, srcElems);
}

MapVariant::MapVariant(const VectorVariant *src, CVarRef k, CVarRef v) :
    m_nextIndex(0), m_keys(NULL) {
  ASSERT(src);
  ASSERT(src->getIndex(k) < 0);
  const HphpVector<Variant *> &srcElems = src->getElems();
  m_nextIndex = srcElems.size();
  for (int i = 0; i < m_nextIndex; i++) {
    m_map[Variant((int64)i)] = i;
  }
  m_elems.reserve(m_nextIndex + 1);
  ArrayFuncs::append(m_elems, srcElems);
  insertKey(k.toKey());
  m_elems.push_back(ArrayFuncs::element(v));
}

MapVariant::MapVariant(const VectorVariant *src, const MapVariant *elems,
                       ArrayOp op) :
    m_nextIndex(0), m_keys(NULL) {
  ASSERT(src && elems);
  m_nextIndex = src->getElems().size();
  for (int i = 0; i < m_nextIndex; i++) {
    m_map[Variant((int64)i)] = i;
  }
  m_elems.reserve(m_nextIndex);
  ArrayFuncs::append(m_elems, src->getElems());
  merge(elems, op);

}

MapVariant::MapVariant(const MapVariant *src, CVarRef v) :
    m_nextIndex(0), m_keys(NULL) {
  ASSERT(src);
  m_map = src->m_map;
  m_nextIndex = src->m_nextIndex;
  m_elems.reserve(src->m_elems.size() + 1);
  ArrayFuncs::append(m_elems, src->m_elems);
  appendKey();
  m_elems.push_back(ArrayFuncs::element(v));
}

MapVariant::MapVariant(const MapVariant *src, CVarRef k, CVarRef v) :
    m_nextIndex(0), m_keys(NULL) {
  ASSERT(src);
  m_map = src->m_map;
  m_nextIndex = src->m_nextIndex;
  int index = src->getIndex(k);
  if (index >= 0) {
    ArrayFuncs::append(m_elems, src->m_elems);
    ArrayFuncs::set(m_elems, index, ArrayFuncs::element(v));
  } else {
    m_elems.reserve(src->m_elems.size() + 1);
    ArrayFuncs::append(m_elems, src->m_elems);
    insertKey(k.toKey());
    m_elems.push_back(ArrayFuncs::element(v));
  }
}

MapVariant::MapVariant(const MapVariant *src, const ArrayData *elems,
                       ArrayOp op) :
    m_nextIndex(0), m_keys(NULL) {
  ASSERT(src && elems);

  const MapVariant *mapVariant = dynamic_cast<const MapVariant *>(elems);
  if (mapVariant) {
    m_map = src->m_map;
    m_nextIndex = src->m_nextIndex;
    m_elems.reserve(src->m_elems.size() + mapVariant->m_elems.size());
    ArrayFuncs::append(m_elems, src->m_elems);
    if (op == Merge)
      renumber();
    merge(mapVariant, op);
    return;
  }

  const VectorVariant *vecVariant = dynamic_cast<const VectorVariant *>(elems);
  if (vecVariant) {
    m_map = src->m_map;
    m_nextIndex = src->m_nextIndex;
    m_elems.reserve(src->m_elems.size() + vecVariant->getElems().size());
    ArrayFuncs::append(m_elems, src->m_elems);
    if (op == Merge)
      renumber();
    merge(vecVariant, op);
    return;
  }

  ASSERT(false);
}

MapVariant::~MapVariant() {
  for (int i = m_elems.size() - 1; i >= 0; --i) {
    ArrayFuncs::release(m_elems[i]);
  }
  delete m_keys;
}

///////////////////////////////////////////////////////////////////////////////
// implementing ArrayData

Variant MapVariant::getKey(ssize_t pos) const {
  ASSERT(pos != ArrayData::invalid_index && pos < m_elems.size());
  const std::vector<Variant> &keys = getKeyVector();
  ASSERT((ssize_t)keys.size() == m_elems.size());
  return keys[pos];
}

Variant MapVariant::get(int64 k, int64 prehash /* = -1 */,
                        bool error /* = false */) const {
  return get(Variant(k), prehash);
}

Variant MapVariant::get(litstr k, int64 prehash /* = -1 */,
                        bool error /* = false */) const {
  return get(Variant(k), prehash);
}

Variant MapVariant::get(CStrRef k, int64 prehash /* = -1 */,
                        bool error /* = false */) const {
  return get(Variant(k), prehash);
}

Variant MapVariant::get(CVarRef k, int64 prehash /* = -1 */,
                        bool error /* = false */) const {
  int index = getIndex(k, prehash);
  if (index >= 0) {
    return *m_elems[index];
  }
  if (error) {
    raise_notice("Undefined index: %s", k.toString().data());
  }
  return null;
}

bool MapVariant::exists(int64 k, int64 prehash /* = -1 */) const {
  return getIndex(k, prehash) >= 0;
}

bool MapVariant::exists(litstr k, int64 prehash /* = -1 */) const {
  return getIndex(k, prehash) >= 0;
}

bool MapVariant::exists(CStrRef k, int64 prehash /* = -1 */) const {
  return getIndex(k, prehash) >= 0;
}

bool MapVariant::exists(CVarRef k, int64 prehash /* = -1 */) const {
  return getIndex(k, prehash) >= 0;
}

bool MapVariant::idxExists(ssize_t idx) const {
  return idx != ArrayData::invalid_index;
}

ssize_t MapVariant::getIndex(CVarRef k, int64 prehash /* = -1 */) const {
  int index;
  if (m_map.find(k, index, prehash)) {
    ASSERT(index >= 0 && index < (int64)m_elems.size());
    return (ssize_t)index;
  }
  return ArrayData::invalid_index;
}

ArrayData *MapVariant::set(int64 k, CVarRef v, bool copy, int64 prehash /* = -1 */) {
  return set(Variant(k), v, copy, prehash);
}

ArrayData *MapVariant::set(litstr k, CVarRef v, bool copy, int64 prehash /* = -1 */) {
  return set(Variant(k), v, copy, prehash);
}

ArrayData *MapVariant::set(CStrRef k, CVarRef v, bool copy,
                    int64 prehash /* = -1 */) {
  return set(Variant(k), v, copy, prehash);
}

ArrayData *MapVariant::set(CVarRef k, CVarRef v, bool copy,
                    int64 prehash /* = -1 */) {
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

const std::vector<Variant> &MapVariant::getKeyVector() const {
  if (m_keys == NULL) {
    m_keys = new std::vector<Variant>();
    m_keys->resize(m_map.size());
    for (HphpMapVariantToInt::const_iterator iter = m_map.begin();
         iter != m_map.end(); ++iter) {
      ASSERT(iter->value() >= 0 && iter->value() < (int)m_map.size());
      (*m_keys)[iter->value()] = iter->key();
    }
    // if this assertion is triggered, our m_map is messed up
    ASSERT((ssize_t)m_keys->size() == m_elems.size());
  }
  // if this assertion is triggered, we didn't call resetKeyVector() somewhere
  ASSERT((ssize_t)m_keys->size() == m_elems.size());
  return *m_keys;
}

void MapVariant::resetKeyVector() const {
  if (m_keys) {
    delete m_keys;
    m_keys = NULL;
  }
}

int MapVariant::insertKey(CVarRef key, int64 prehash /* = -1 */) {
  int res;
  ASSERT(key.isInteger() || key.isString());
  if (m_map.insert(key, m_elems.size(), res, prehash)) {
    if (m_keys) {
      m_keys->push_back(key);
    }
    if (key.isInteger()) {
      int64 index = key.toInt64();
      if (index + 1 > m_nextIndex) m_nextIndex = index + 1;
    }
  }
  return res;
}

void MapVariant::insertKeyAtPos(int pos) {
  const std::vector<Variant> &keys = getKeyVector();

  HphpMapVariantToInt newmap;
  m_nextIndex = 0;
  for (int i = 0; i <= (int)keys.size(); i++) {
    if (i == pos) {
      Variant newkey((int64)m_nextIndex++);
      newmap[newkey] = pos;
    } else {
      CVarRef key = keys[i > pos ? i-1 : i];
      int value = m_map[key];
      if (value >= pos) value++;
      if (key.isInteger() && key.toInt64() >= 0) {
        Variant newkey((int64)m_nextIndex++);
        newmap[newkey] = value;
      } else {
        newmap[key] = value;
      }
    }
  }
  m_map.swap(newmap);

  resetKeyVector();
}

void MapVariant::appendKey() {
  Variant key((int64)m_nextIndex++);
  m_map[key] = m_elems.size();
  if (m_keys) {
    m_keys->push_back(key);
  }
}

void MapVariant::removeKey(CVarRef key, int index, int64 prehash) {
  if (m_keys) {
    m_keys->erase(m_keys->begin() + index);
  }
  for (HphpMapVariantToInt::iterator iter = m_map.begin(); iter != m_map.end();
       ++iter) {
    if (iter->value() > index) {
      --iter->lvalue();
    }
  }
  m_map.erase(key, prehash);
}

void MapVariant::renumber() {
  const std::vector<Variant> &keys = getKeyVector();

  HphpMapVariantToInt newmap;
  m_nextIndex = 0;
  for (unsigned int i = 0; i < keys.size(); i++) {
    CVarRef key = keys[i];
    if (key.isInteger()) {
      Variant newkey((int64)m_nextIndex++);
      newmap[newkey] = m_map[key];
    } else {
      newmap[key] = m_map[key];
    }
  }
  m_map.swap(newmap);

  resetKeyVector();
}

ssize_t MapVariant::size() const {
  return m_elems.size();
}

Variant MapVariant::getValue(ssize_t pos) const {
  ASSERT(pos >= 0 && pos < m_elems.size());
  return *m_elems[pos];
}

void MapVariant::fetchValue(ssize_t pos, Variant & v) const {
  ASSERT(pos >= 0 && pos < m_elems.size());
  v = *m_elems[pos];
}

CVarRef MapVariant::getValueRef(ssize_t pos) const {
  ASSERT(pos >= 0 && pos < m_elems.size());
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
                            int64 prehash /* = -1 */,
                            bool checkExist /* = false */) {
  return lval(Variant(k), ret, copy, prehash, checkExist);
}

ArrayData *MapVariant::lval(litstr k, Variant *&ret, bool copy,
                            int64 prehash /* = -1 */,
                            bool checkExist /* = false */) {
  return lval(Variant(k), ret, copy, prehash, checkExist);
}

ArrayData *MapVariant::lval(CStrRef k, Variant *&ret, bool copy,
                            int64 prehash /* = -1 */,
                            bool checkExist /* = false */) {
  return lval(Variant(k), ret, copy, prehash, checkExist);
}

ArrayData *MapVariant::lval(CVarRef k, Variant *&ret, bool copy,
                            int64 prehash /* = -1 */,
                            bool checkExist /* = false */) {
  // XXX not using checkExist yet
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
    if (copy) {
      MapVariant * ret = NEW(MapVariant)(this);
      ret->remove(k, false);
      return ret;
    }
    m_elems[index]->release();
    m_elems.remove(index);
    removeKey(k, index, prehash);
    if (index < m_pos) m_pos--;
  }
  if (copy) {
    return NEW(MapVariant)(this);
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
  const SharedMap *mapShared = dynamic_cast<const SharedMap *>(elems);
  if (mapShared) {
    MapVariant *escalated = mapShared->escalateToMapVariant();
    MapVariant *ret = NULL;
    if (copy) {
      ret = NEW(MapVariant)(this, escalated, op);
    } else {
      merge(escalated, op);
    }
    DELETE(MapVariant)(escalated);
    return ret;
  }

  if (copy) {
    return NEW(MapVariant)(this, elems, op);
  }

  const MapVariant *mapVariant = dynamic_cast<const MapVariant *>(elems);
  if (mapVariant) {
    merge(mapVariant, op);
    return NULL;
  }

  const VectorVariant *vec = dynamic_cast<const VectorVariant *>(elems);
  if (vec) {
    merge(vec, op);
    return NULL;
  }

  ASSERT(false);
  return NULL;
}

ArrayData *MapVariant::prepend(CVarRef v, bool copy) {
  if (copy) {
    ArrayData* ret = NEW(MapVariant)(this);
    ret->prepend(v, false);
    return ret;
  }

  insertKeyAtPos(0);
  m_elems.insert(0, ArrayFuncs::element(v));
  return NULL;
}

void MapVariant::merge(const MapVariant *srcMap, ArrayOp op) {
  const HphpVector<Variant*> &elems = srcMap->m_elems;
  const std::vector<Variant> &keys = srcMap->getKeyVector();
  unsigned int size = keys.size();
  if (op == Plus) {
    m_elems.reserve(m_elems.size() + elems.size());
    for (unsigned int i = 0; i < size; i++) {
      CVarRef key = keys[i];
      int index = getIndex(key);
      if (index < 0) {
        insertKey(key);
        Variant * elem;
        ArrayFuncs::element(elem, elems[i]);
        m_elems.push_back(elem);
      }
    }
  } else {
    ASSERT(op == Merge);
    for (unsigned int i = 0; i < size; i++) {
      CVarRef key = keys[i];
      Variant * elem;
      ArrayFuncs::element(elem, elems[i]);
      if (key.isNumeric()) {
        appendKey();
        m_elems.push_back(elem);
      } else {
        int index = getIndex(key);
        if (index < 0) {
          insertKey(key);
          m_elems.push_back(elem);
        } else {
          ArrayFuncs::set(m_elems, index, elem);
        }
      }
    }
  }
}

void MapVariant::merge(const VectorVariant * vec, ArrayOp op) {
  const HphpVector<Variant*> &elems = vec->getElems();
  unsigned int size = elems.size();
  if (op == Plus) {
    m_elems.reserve(m_elems.size() + size);
    for (unsigned int i = 0; i < size; i++) {
      Variant key((int64)i);
      int index = getIndex(key);
      if (index < 0) {
        insertKey(key);
        Variant * elem;
        ArrayFuncs::element(elem, elems[i]);
        m_elems.push_back(elem);
      }
    }
  } else {
    ASSERT(op == Merge);
    for (unsigned int i = 0; i < size; i++) {
      appendKey();
      Variant * elem;
      ArrayFuncs::element(elem, elems[i]);
      m_elems.push_back(elem);
    }
  }
}

void MapVariant::onSetStatic() {
  for (HphpMapVariantToInt::const_iterator iter = m_map.begin();
       iter != m_map.end(); ++iter) {
    iter->key().setStatic();
  }
  // populate m_keys so that it won't be populated again by different threads
  getKeyVector();
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
  if (m_pos == ArrayData::invalid_index || m_pos >= m_elems.size()) {
    return false;
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
}
