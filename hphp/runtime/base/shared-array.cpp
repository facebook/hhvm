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

#include "hphp/runtime/base/shared-array.h"
#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/runtime-error.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

HOT_FUNC
CVarRef SharedArray::getValueRef(ssize_t pos) const {
  SharedVariant *sv = m_arr->getValue(pos);
  DataType t = sv->getType();
  if (!IS_REFCOUNTED_TYPE(t)) return sv->asCVarRef();
  if (LIKELY(m_localCache != nullptr)) {
    assert(unsigned(pos) < m_arr->arrCap());
    TypedValue* tv = &m_localCache[pos];
    if (tv->m_type != KindOfUninit) return tvAsCVarRef(tv);
  } else {
    static_assert(KindOfUninit == 0, "must be 0 since we use smart_calloc");
    unsigned cap = m_arr->arrCap();
    m_localCache = (TypedValue*) smart_calloc(cap, sizeof(TypedValue));
  }
  TypedValue* tv = &m_localCache[pos];
  tvAsVariant(tv) = sv->toLocal();
  assert(tv->m_type != KindOfUninit);
  return tvAsCVarRef(tv);
}

CVarRef SharedArray::GetValueRef(const ArrayData* ad, ssize_t pos) {
  return asSharedArray(ad)->getValueRef(pos);
}

SharedVariant* SharedArray::GetSharedVariant(const ArrayData* ad) {
  auto a = asSharedArray(ad);
  if (a->m_arr->shouldCache()) return nullptr;
  return a->m_arr;
}

ALWAYS_INLINE SharedArray::~SharedArray() {
  if (m_localCache) {
    for (TypedValue* tv = m_localCache, *end = tv + m_arr->arrCap();
         tv < end; ++tv) {
      tvRefcountedDecRef(tv);
    }
    smart_free(m_localCache);
  }
  m_arr->decRef();
}

HOT_FUNC
void SharedArray::Release(ArrayData* ad) {
  auto const smap = asSharedArray(ad);
  smap->~SharedArray();
  MM().smartFreeSize(smap, sizeof(SharedArray));
}

inline SharedArray* SharedArray::asSharedArray(ArrayData* ad) {
  assert(ad->kind() == kSharedKind);
  return static_cast<SharedArray*>(ad);
}

inline const SharedArray* SharedArray::asSharedArray(const ArrayData* ad) {
  assert(ad->kind() == kSharedKind);
  return static_cast<const SharedArray*>(ad);
}

bool SharedArray::IsVectorData(const ArrayData* ad) {
  auto a = asSharedArray(ad);
  const auto n = a->size();
  for (ssize_t i = 0; i < n; i++) {
    if (a->getIndex(i) != i) return false;
  }
  return true;
}

bool SharedArray::ExistsStr(const ArrayData* ad, const StringData* k) {
  auto a = asSharedArray(ad);
  return a->getIndex(k) != -1;
}

bool SharedArray::ExistsInt(const ArrayData* ad, int64_t k) {
  return asSharedArray(ad)->getIndex(k) != -1;
}

ssize_t SharedArray::getIndex(int64_t k) const {
  return m_arr->getIndex(k);
}

ssize_t SharedArray::getIndex(const StringData* k) const {
  return m_arr->getIndex(k);
}

/* if a2 is modified copy of a1 (i.e. != a1), then release a1 and return a2 */
static inline ArrayData* releaseIfCopied(ArrayData* a1, ArrayData* a2) {
  if (a1 != a2) a1->release();
  return a2;
}

ArrayData *SharedArray::LvalInt(ArrayData* ad, int64_t k, Variant *&ret,
                              bool copy) {
  ArrayData *escalated = Escalate(ad);
  return releaseIfCopied(escalated, escalated->lval(k, ret, false));
}

ArrayData *SharedArray::LvalStr(ArrayData* ad, StringData* k, Variant *&ret,
                              bool copy) {
  ArrayData *escalated = Escalate(ad);
  return releaseIfCopied(escalated, escalated->lval(k, ret, false));
}

ArrayData *SharedArray::LvalNew(ArrayData* ad, Variant *&ret, bool copy) {
  ArrayData *escalated = Escalate(ad);
  return releaseIfCopied(escalated, escalated->lvalNew(ret, false));
}

ArrayData*
SharedArray::SetInt(ArrayData* ad, int64_t k, CVarRef v, bool copy) {
  ArrayData *escalated = Escalate(ad);
  return releaseIfCopied(escalated, escalated->set(k, v, false));
}

ArrayData*
SharedArray::SetStr(ArrayData* ad, StringData* k, CVarRef v, bool copy) {
  ArrayData *escalated = Escalate(ad);
  return releaseIfCopied(escalated, escalated->set(k, v, false));
}

ArrayData*
SharedArray::SetRefInt(ArrayData* ad, int64_t k, CVarRef v, bool copy) {
  ArrayData *escalated = Escalate(ad);
  return releaseIfCopied(escalated, escalated->setRef(k, v, false));
}

ArrayData*
SharedArray::SetRefStr(ArrayData* ad, StringData* k, CVarRef v, bool copy) {
  ArrayData *escalated = Escalate(ad);
  return releaseIfCopied(escalated, escalated->setRef(k, v, false));
}

ArrayData *SharedArray::RemoveInt(ArrayData* ad, int64_t k, bool copy) {
  ArrayData *escalated = Escalate(ad);
  return releaseIfCopied(escalated, escalated->remove(k, false));
}

ArrayData*
SharedArray::RemoveStr(ArrayData* ad, const StringData* k, bool copy) {
  ArrayData *escalated = Escalate(ad);
  return releaseIfCopied(escalated, escalated->remove(k, false));
}

ArrayData* SharedArray::Copy(const ArrayData* ad) {
  return Escalate(ad);
}

ArrayData *SharedArray::Append(ArrayData* ad, CVarRef v, bool copy) {
  ArrayData *escalated = Escalate(ad);
  return releaseIfCopied(escalated, escalated->append(v, false));
}

ArrayData*
SharedArray::AppendRef(ArrayData* ad, CVarRef v, bool copy) {
  ArrayData *escalated = Escalate(ad);
  return releaseIfCopied(escalated, escalated->appendRef(v, false));
}

ArrayData*
SharedArray::AppendWithRef(ArrayData* ad, CVarRef v, bool copy) {
  ArrayData *escalated = Escalate(ad);
  return releaseIfCopied(escalated, escalated->appendWithRef(v, false));
}

ArrayData* SharedArray::Plus(ArrayData* ad, const ArrayData *elems) {
  // XXX: this could be more efficient.
  Array escalated = Escalate(ad);
  return escalated->plus(elems);
}

ArrayData* SharedArray::Merge(ArrayData* ad, const ArrayData *elems) {
  // XXX: this could be more efficient.
  Array escalated = Escalate(ad);
  return escalated->merge(elems);
}

ArrayData *SharedArray::Prepend(ArrayData* ad, CVarRef v, bool copy) {
  ArrayData *escalated = Escalate(ad);
  return releaseIfCopied(escalated, escalated->prepend(v, false));
}

ArrayData *SharedArray::Escalate(const ArrayData* ad) {
  auto smap = asSharedArray(ad);
  auto ret = smap->m_arr->loadElems(*smap);
  assert(!ret->isStatic());
  return ret;
}

TypedValue* SharedArray::NvGetInt(const ArrayData* ad, int64_t k) {
  auto a = asSharedArray(ad);
  auto index = a->getIndex(k);
  if (index == -1) return nullptr;
  return (TypedValue*)&a->getValueRef(index);
}

TypedValue* SharedArray::NvGetStr(const ArrayData* ad, const StringData* key) {
  auto a = asSharedArray(ad);
  auto index = a->getIndex(key);
  if (index == -1) return nullptr;
  return (TypedValue*)&a->getValueRef(index);
}

void SharedArray::NvGetKey(const ArrayData* ad, TypedValue* out, ssize_t pos) {
  auto a = asSharedArray(ad);
  Variant k = a->m_arr->getKey(pos);
  TypedValue* tv = k.asTypedValue();
  // copy w/out clobbering out->_count.
  out->m_type = tv->m_type;
  out->m_data.num = tv->m_data.num;
  if (tv->m_type != KindOfInt64) out->m_data.pstr->incRefCount();
}

ArrayData* SharedArray::EscalateForSort(ArrayData* ad) {
  auto a = asSharedArray(ad);
  auto ret = a->m_arr->loadElems(*a);
  assert(!ret->isStatic());
  return ret;
}

ssize_t SharedArray::IterBegin(const ArrayData* ad) {
  auto a = asSharedArray(ad);
  return a->m_size > 0 ? 0 : invalid_index;
}

ssize_t SharedArray::IterEnd(const ArrayData* ad) {
  auto a = asSharedArray(ad);
  auto n = a->m_size;
  return n > 0 ? ssize_t(n - 1) : invalid_index;
}

ssize_t SharedArray::IterAdvance(const ArrayData* ad, ssize_t prev) {
  auto a = asSharedArray(ad);
  assert(prev >= 0 && prev < a->m_size);
  ssize_t next = prev + 1;
  return next < a->m_size ? next : invalid_index;
}

ssize_t SharedArray::IterRewind(const ArrayData* ad, ssize_t prev) {
  assert(prev >= 0 && prev < asSharedArray(ad)->m_size);
  ssize_t next = prev - 1;
  return next >= 0 ? next : invalid_index;
}

bool SharedArray::ValidFullPos(const ArrayData* ad, const FullPos& fp) {
  assert(fp.getContainer() == ad);
  return false;
}

bool SharedArray::AdvanceFullPos(ArrayData* ad, FullPos& fp) {
  return false;
}

void SharedArray::getChildren(std::vector<TypedValue *> &out) {
  if (m_localCache) {
    TypedValue *localCacheEnd = m_localCache + m_size;
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
