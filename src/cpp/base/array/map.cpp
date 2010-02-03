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

#include <cpp/base/array/map.h>
#include <cpp/base/array/map_variant.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// constructors and destructor

Map::Map() : m_nextIndex(0), m_keys(NULL) {
}

Map::Map(const Map *src) : ArrayData(src), m_nextIndex(0), m_keys(NULL) {
  m_map = src->m_map;
  m_nextIndex = src->m_nextIndex;
}

Map::~Map() {
  delete m_keys;
}

///////////////////////////////////////////////////////////////////////////////
// implementing ArrayData

Variant Map::getKey(ssize_t pos) const {
  ASSERT(pos != ArrayData::invalid_index && pos < size());
  const vector<Variant> &keys = getKeyVector();
  ASSERT((ssize_t)keys.size() == size());
  return keys[pos];
}

Variant Map::get(int64 k, int64 prehash /* = -1 */) const {
  return get(Variant(k), prehash);
}

Variant Map::get(litstr k, int64 prehash /* = -1 */) const {
  return get(Variant(k), prehash);
}

Variant Map::get(CStrRef k, int64 prehash /* = -1 */) const {
  return get(Variant(k), prehash);
}

Variant Map::get(CVarRef k, int64 prehash /* = -1 */) const {
  int index = getIndex(k, prehash);
  if (index >= 0) {
    return getImpl(index);
  } else {
    return null;
  }
}

bool Map::exists(int64 k, int64 prehash /* = -1 */) const {
  return getIndex(k, prehash) >= 0;
}

bool Map::exists(litstr k, int64 prehash /* = -1 */) const {
  return getIndex(k, prehash) >= 0;
}

bool Map::exists(CStrRef k, int64 prehash /* = -1 */) const {
  return getIndex(k, prehash) >= 0;
}

bool Map::exists(CVarRef k, int64 prehash /* = -1 */) const {
  return getIndex(k, prehash) >= 0;
}

bool Map::idxExists(ssize_t idx) const {
  return idx != ArrayData::invalid_index;
}

ssize_t Map::getIndex(CVarRef k, int64 prehash /* = -1 */) const {
  int index;
  if (m_map.find(k, index, prehash)) {
    ASSERT(index >= 0 && index < size());
    return (ssize_t)index;
  }
  return ArrayData::invalid_index;
}


ArrayData *Map::set(int64 k, CVarRef v, bool copy, int64 prehash /* = -1 */) {
  return setImpl(Variant(k), v, copy, prehash);
}

ArrayData *Map::set(litstr k, CVarRef v, bool copy, int64 prehash /* = -1 */) {
  return setImpl(Variant(k), v, copy, prehash);
}

ArrayData *Map::set(CStrRef k, CVarRef v, bool copy,
                    int64 prehash /* = -1 */) {
  return setImpl(Variant(k), v, copy, prehash);
}

ArrayData *Map::set(CVarRef k, CVarRef v, bool copy,
                    int64 prehash /* = -1 */) {
  return setImpl(k, v, copy, prehash);
}

///////////////////////////////////////////////////////////////////////////////
// helpers

const vector<Variant> &Map::getKeyVector() const {
  if (m_keys == NULL) {
    m_keys = new vector<Variant>();
    m_keys->resize(m_map.size());
    for (HphpMapVariantToInt::const_iterator iter = m_map.begin();
         iter != m_map.end(); ++iter) {
      ASSERT(iter->value() >= 0 && iter->value() < (int)m_map.size());
      (*m_keys)[iter->value()] = iter->key();
    }
    // if this assertion is triggered, our m_map is messed up
    ASSERT((ssize_t)m_keys->size() == size());
  }
  // if this assertion is triggered, we didn't call resetKeyVector() somewhere
  ASSERT((ssize_t)m_keys->size() == size());
  return *m_keys;
}

void Map::resetKeyVector() const {
  if (m_keys) {
    delete m_keys;
    m_keys = NULL;
  }
}

int Map::insertKey(CVarRef key, int64 prehash /* = -1 */) {
  int res;
  ASSERT(key.isInteger() || key.isString());
  if (m_map.insert(key, size(), res, prehash)) {
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

void Map::insertKey(int pos) {
  const vector<Variant> &keys = getKeyVector();

  HphpMapVariantToInt newmap;
  m_nextIndex = 0;
  for (int i = 0; i <= (int)keys.size(); i++) {
    if (i == pos) {
      Variant newkey = (int64)m_nextIndex++;
      newmap[newkey] = pos;
    } else {
      CVarRef key = keys[i > pos ? i-1 : i];
      int value = m_map[key];
      if (value >= pos) value++;
      if (key.isInteger() && key.toInt64() >= 0) {
        Variant newkey = (int64)m_nextIndex++;
        newmap[newkey] = value;
      } else {
        newmap[key] = value;
      }
    }
  }
  m_map.swap(newmap);

  resetKeyVector();
}

void Map::appendKey() {
  Variant key = (int64)m_nextIndex++;
  m_map[key] = size();
  if (m_keys) {
    m_keys->push_back(key);
  }
}

void Map::removeKey(CVarRef key, int index, int64 prehash) {
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

void Map::renumber() {
  const vector<Variant> &keys = getKeyVector();

  HphpMapVariantToInt newmap;
  m_nextIndex = 0;
  for (unsigned int i = 0; i < keys.size(); i++) {
    CVarRef key = keys[i];
    if (key.isInteger()) {
      Variant newkey = (int64)m_nextIndex++;
      newmap[newkey] = m_map[key];
    } else {
      newmap[key] = m_map[key];
    }
  }
  m_map.swap(newmap);

  resetKeyVector();
}

void Map::onSetStatic() {
  for (HphpMapVariantToInt::const_iterator iter = m_map.begin();
       iter != m_map.end(); ++iter) {
    iter->key().setStatic();
  }

  // populate m_keys so that it won't be populated again by different threads
  getKeyVector();
}

///////////////////////////////////////////////////////////////////////////////
}
