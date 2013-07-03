/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/shared/shared_map.h"
#include "hphp/runtime/base/type_conversions.h"
#include "hphp/runtime/base/array/array_iterator.h"
#include "hphp/runtime/base/array/array_init.h"
#include "hphp/runtime/base/runtime_option.h"
#include "hphp/runtime/base/runtime_error.h"

namespace HPHP {

IMPLEMENT_SMART_ALLOCATION_HOT(SharedMap);
///////////////////////////////////////////////////////////////////////////////
HOT_FUNC
CVarRef SharedMap::getValueRef(ssize_t pos) const {
  SharedVariant *sv = getValueImpl(pos);
  DataType t = sv->getType();
  if (!IS_REFCOUNTED_TYPE(t)) return sv->asCVarRef();
  if (LIKELY(m_localCache != nullptr)) {
    assert(unsigned(pos) < size());
    TypedValue* tv = &m_localCache[pos];
    if (tv->m_type != KindOfUninit) return tvAsCVarRef(tv);
  } else {
    static_assert(KindOfUninit == 0, "must be 0 since we use smart_calloc");
    m_localCache = (TypedValue*) smart_calloc(size(), sizeof(TypedValue));
  }
  TypedValue* tv = &m_localCache[pos];
  tvAsVariant(tv) = sv->toLocal();
  assert(tv->m_type != KindOfUninit);
  return tvAsCVarRef(tv);
}

HOT_FUNC
SharedMap::~SharedMap() {
  if (m_localCache) {
    for (TypedValue* tv = m_localCache, *end = tv + size();
         tv < end; ++tv) {
      tvRefcountedDecRef(tv);
    }
    smart_free(m_localCache);
  }
}

ssize_t SharedMap::getIndex(const StringData* k) const {
  if (isVector()) return -1;
  return m_map->indexOf(k);
}

ssize_t SharedMap::getIndex(int64_t k) const {
  if (isVector()) {
    if (k < 0 || (size_t)k >= m_vec->m_size) return -1;
    return k;
  }
  return m_map->indexOf(k);
}

bool SharedMap::isVectorData() const {
  if (isVector()) return true;
  const auto n = size();
  for (ssize_t i = 0; i < n; i++) {
    if (m_map->indexOf(i) != i) return false;
  }
  return true;
}

bool SharedMap::exists(const StringData* k) const {
  if (isVector()) return false;
  return m_map->indexOf(k) != -1;
}

bool SharedMap::exists(int64_t k) const {
  return getIndex(k) != -1;
  if (isVector()) {
    if (k < 0 || (size_t)k >= m_vec->m_size) return false;
    return true;
  }
  return m_map->indexOf(k) != -1;
}

/* if a2 is modified copy of a1 (i.e. != a1), then release a1 and return a2 */
inline ArrayData* releaseIfCopied(ArrayData* a1, ArrayData* a2) {
  if (a1 != a2) a1->release();
  return a2;
}

ArrayData *SharedMap::lval(int64_t k, Variant *&ret, bool copy,
                           bool checkExist /* = false */) {
  ArrayData *escalated = SharedMap::escalate();
  return releaseIfCopied(escalated, escalated->lval(k, ret, false));
}

ArrayData *SharedMap::lval(StringData* k, Variant *&ret, bool copy,
                           bool checkExist /* = false */) {
  ArrayData *escalated = SharedMap::escalate();
  return releaseIfCopied(escalated, escalated->lval(k, ret, false));
}

ArrayData *SharedMap::lvalNew(Variant *&ret, bool copy) {
  ArrayData *escalated = SharedMap::escalate();
  return releaseIfCopied(escalated, escalated->lvalNew(ret, false));
}

ArrayData *SharedMap::set(int64_t k, CVarRef v, bool copy) {
  ArrayData *escalated = SharedMap::escalate();
  return releaseIfCopied(escalated, escalated->set(k, v, false));
}

ArrayData *SharedMap::set(StringData* k, CVarRef v, bool copy) {
  ArrayData *escalated = SharedMap::escalate();
  return releaseIfCopied(escalated, escalated->set(k, v, false));
}

ArrayData *SharedMap::setRef(int64_t k, CVarRef v, bool copy) {
  ArrayData *escalated = SharedMap::escalate();
  return releaseIfCopied(escalated, escalated->setRef(k, v, false));
}

ArrayData *SharedMap::setRef(StringData* k, CVarRef v, bool copy) {
  ArrayData *escalated = SharedMap::escalate();
  return releaseIfCopied(escalated, escalated->setRef(k, v, false));
}

ArrayData *SharedMap::remove(int64_t k, bool copy) {
  ArrayData *escalated = SharedMap::escalate();
  return releaseIfCopied(escalated, escalated->remove(k, false));
}

ArrayData *SharedMap::remove(const StringData* k, bool copy) {
  ArrayData *escalated = SharedMap::escalate();
  return releaseIfCopied(escalated, escalated->remove(k, false));
}

ArrayData *SharedMap::copy() const {
  return SharedMap::escalate();
}

ArrayData *SharedMap::append(CVarRef v, bool copy) {
  ArrayData *escalated = SharedMap::escalate();
  return releaseIfCopied(escalated, escalated->append(v, false));
}

ArrayData *SharedMap::appendRef(CVarRef v, bool copy) {
  ArrayData *escalated = SharedMap::escalate();
  return releaseIfCopied(escalated, escalated->appendRef(v, false));
}

ArrayData *SharedMap::appendWithRef(CVarRef v, bool copy) {
  ArrayData *escalated = SharedMap::escalate();
  return releaseIfCopied(escalated, escalated->appendWithRef(v, false));
}

ArrayData *SharedMap::plus(const ArrayData *elems, bool copy) {
  ArrayData *escalated = SharedMap::escalate();
  return releaseIfCopied(escalated, escalated->plus(elems, false));
}

ArrayData *SharedMap::merge(const ArrayData *elems, bool copy) {
  ArrayData *escalated = SharedMap::escalate();
  return releaseIfCopied(escalated, escalated->merge(elems, false));
}

ArrayData *SharedMap::prepend(CVarRef v, bool copy) {
  ArrayData *escalated = SharedMap::escalate();
  return releaseIfCopied(escalated, escalated->prepend(v, false));
}

ArrayData *SharedMap::escalate() const {
  ArrayData *ret = loadElems();
  assert(!ret->isStatic());
  return ret;
}

TypedValue* SharedMap::nvGet(int64_t k) const {
  int index = getIndex(k);
  if (index == -1) return nullptr;
  return (TypedValue*)&getValueRef(index);
}

TypedValue* SharedMap::nvGet(const StringData* key) const {
  int index = getIndex(key);
  if (index == -1) return nullptr;
  return (TypedValue*)&getValueRef(index);
}

void SharedMap::nvGetKey(TypedValue* out, ssize_t pos) {
  Variant k = getKey(pos);
  TypedValue* tv = k.asTypedValue();
  // copy w/out clobbering out->_count.
  out->m_type = tv->m_type;
  out->m_data.num = tv->m_data.num;
  if (tv->m_type != KindOfInt64) out->m_data.pstr->incRefCount();
}

TypedValue* SharedMap::nvGetValueRef(ssize_t pos) {
  return const_cast<TypedValue*>(SharedMap::getValueRef(pos).asTypedValue());
}

TypedValue* SharedMap::nvGetCell(int64_t k) const {
  int index = getIndex(k);
  return index != -1 ? const_cast<Cell*>(getValueRef(index).asCell())
                     : nvGetNotFound(k);
}

TypedValue* SharedMap::nvGetCell(const StringData* key) const {
  int index = getIndex(key);
  return index != -1 ? const_cast<Cell*>(getValueRef(index).asCell())
                     : nvGetNotFound(key);
}

ArrayData* SharedMap::escalateForSort() {
  ArrayData *ret = loadElems(true /* mapInit */);
  assert(!ret->isStatic());
  return ret;
}

ArrayData* SharedMap::loadElems(bool mapInit /* = false */) const {
  uint count = size();
  bool isVec = isVector();

  auto ai =
    mapInit ? ArrayInit(count, ArrayInit::mapInit) :
    isVec ? ArrayInit(count, ArrayInit::vectorInit) :
    ArrayInit(count);

  if (isVec) {
    for (uint i = 0; i < count; i++) {
      ai.set(getValueRef(i));
    }
  } else {
    for (uint i = 0; i < count; i++) {
      ai.add(m_map->getKey(i), getValueRef(i),
             true);
    }
  }
  ArrayData* elems = ai.create();
  if (elems->isStatic()) elems = elems->copy();
  return elems;
}

void SharedMap::getChildren(std::vector<TypedValue *> &out) {
  if (m_localCache) {
    TypedValue *localCacheEnd = m_localCache + size();
    for (TypedValue *tv = m_localCache;
         tv < localCacheEnd;
         ++tv) {
      if (tv->m_type != KindOfUninit) {
        out.push_back(tv);
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
}
