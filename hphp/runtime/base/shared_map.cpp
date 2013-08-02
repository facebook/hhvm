/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1998-2010 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
*/

#include "hphp/runtime/base/shared_map.h"
#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/base/array_iterator.h"
#include "hphp/runtime/base/array_init.h"
#include "hphp/runtime/base/runtime_option.h"
#include "hphp/runtime/base/runtime_error.h"

namespace HPHP {

IMPLEMENT_SMART_ALLOCATION_HOT(SharedMap);
///////////////////////////////////////////////////////////////////////////////
HOT_FUNC
CVarRef SharedMap::getValueRef(ssize_t pos) const {
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

CVarRef SharedMap::GetValueRef(const ArrayData* ad, ssize_t pos) {
  return asSharedMap(ad)->getValueRef(pos);
}

SharedVariant* SharedMap::GetSharedVariant(const ArrayData* ad) {
  auto a = asSharedMap(ad);
  if (a->m_arr->shouldCache()) return nullptr;
  return a->m_arr;
}

HOT_FUNC
SharedMap::~SharedMap() {
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
void SharedMap::Release(ArrayData* ad) {
  asSharedMap(ad)->release();
}

inline SharedMap* SharedMap::asSharedMap(ArrayData* ad) {
  assert(ad->kind() == kSharedKind);
  return static_cast<SharedMap*>(ad);
}

inline const SharedMap* SharedMap::asSharedMap(const ArrayData* ad) {
  assert(ad->kind() == kSharedKind);
  return static_cast<const SharedMap*>(ad);
}

bool SharedMap::IsVectorData(const ArrayData* ad) {
  auto a = asSharedMap(ad);
  const auto n = a->size();
  for (ssize_t i = 0; i < n; i++) {
    if (a->getIndex(i) != i) return false;
  }
  return true;
}

bool SharedMap::ExistsStr(const ArrayData* ad, const StringData* k) {
  auto a = asSharedMap(ad);
  return a->getIndex(k) != -1;
}

bool SharedMap::ExistsInt(const ArrayData* ad, int64_t k) {
  return asSharedMap(ad)->getIndex(k) != -1;
}

ssize_t SharedMap::getIndex(int64_t k) const {
  return m_arr->getIndex(k);
}

ssize_t SharedMap::getIndex(const StringData* k) const {
  return m_arr->getIndex(k);
}

/* if a2 is modified copy of a1 (i.e. != a1), then release a1 and return a2 */
inline ArrayData* releaseIfCopied(ArrayData* a1, ArrayData* a2) {
  if (a1 != a2) a1->release();
  return a2;
}

ArrayData *SharedMap::LvalInt(ArrayData* ad, int64_t k, Variant *&ret,
                              bool copy) {
  ArrayData *escalated = Escalate(ad);
  return releaseIfCopied(escalated, escalated->lval(k, ret, false));
}

ArrayData *SharedMap::LvalStr(ArrayData* ad, StringData* k, Variant *&ret,
                              bool copy) {
  ArrayData *escalated = Escalate(ad);
  return releaseIfCopied(escalated, escalated->lval(k, ret, false));
}

ArrayData *SharedMap::LvalNew(ArrayData* ad, Variant *&ret, bool copy) {
  ArrayData *escalated = Escalate(ad);
  return releaseIfCopied(escalated, escalated->lvalNew(ret, false));
}

ArrayData*
SharedMap::SetInt(ArrayData* ad, int64_t k, CVarRef v, bool copy) {
  ArrayData *escalated = Escalate(ad);
  return releaseIfCopied(escalated, escalated->set(k, v, false));
}

ArrayData*
SharedMap::SetStr(ArrayData* ad, StringData* k, CVarRef v, bool copy) {
  ArrayData *escalated = Escalate(ad);
  return releaseIfCopied(escalated, escalated->set(k, v, false));
}

ArrayData*
SharedMap::SetRefInt(ArrayData* ad, int64_t k, CVarRef v, bool copy) {
  ArrayData *escalated = Escalate(ad);
  return releaseIfCopied(escalated, escalated->setRef(k, v, false));
}

ArrayData*
SharedMap::SetRefStr(ArrayData* ad, StringData* k, CVarRef v, bool copy) {
  ArrayData *escalated = Escalate(ad);
  return releaseIfCopied(escalated, escalated->setRef(k, v, false));
}

ArrayData *SharedMap::AddLvalInt(ArrayData* ad, int64_t k, Variant *&ret,
                                 bool copy) {
  ArrayData *escalated = Escalate(ad);
  return releaseIfCopied(escalated, escalated->addLval(k, ret, false));
}

ArrayData *SharedMap::AddLvalStr(ArrayData* ad, StringData* k, Variant *&ret,
                                 bool copy) {
  ArrayData *escalated = Escalate(ad);
  return releaseIfCopied(escalated, escalated->addLval(k, ret, false));
}

ArrayData *SharedMap::RemoveInt(ArrayData* ad, int64_t k, bool copy) {
  ArrayData *escalated = Escalate(ad);
  return releaseIfCopied(escalated, escalated->remove(k, false));
}

ArrayData *SharedMap::RemoveStr(ArrayData* ad, const StringData* k, bool copy) {
  ArrayData *escalated = Escalate(ad);
  return releaseIfCopied(escalated, escalated->remove(k, false));
}

ArrayData* SharedMap::Copy(const ArrayData* ad) {
  return Escalate(ad);
}

ArrayData *SharedMap::Append(ArrayData* ad, CVarRef v, bool copy) {
  ArrayData *escalated = Escalate(ad);
  return releaseIfCopied(escalated, escalated->append(v, false));
}

ArrayData *SharedMap::AppendRef(ArrayData* ad, CVarRef v, bool copy) {
  ArrayData *escalated = Escalate(ad);
  return releaseIfCopied(escalated, escalated->appendRef(v, false));
}

ArrayData *SharedMap::AppendWithRef(ArrayData* ad, CVarRef v, bool copy) {
  ArrayData *escalated = Escalate(ad);
  return releaseIfCopied(escalated, escalated->appendWithRef(v, false));
}

ArrayData *SharedMap::Plus(ArrayData* ad, const ArrayData *elems, bool copy) {
  ArrayData *escalated = Escalate(ad);
  return releaseIfCopied(escalated, escalated->plus(elems, false));
}

ArrayData *SharedMap::Merge(ArrayData* ad, const ArrayData *elems, bool copy) {
  ArrayData *escalated = Escalate(ad);
  return releaseIfCopied(escalated, escalated->merge(elems, false));
}

ArrayData *SharedMap::Prepend(ArrayData* ad, CVarRef v, bool copy) {
  ArrayData *escalated = Escalate(ad);
  return releaseIfCopied(escalated, escalated->prepend(v, false));
}

ArrayData *SharedMap::Escalate(const ArrayData* ad) {
  auto smap = asSharedMap(ad);
  auto ret = smap->m_arr->loadElems(*smap);
  assert(!ret->isStatic());
  return ret;
}

TypedValue* SharedMap::NvGetInt(const ArrayData* ad, int64_t k) {
  auto a = asSharedMap(ad);
  auto index = a->getIndex(k);
  if (index == -1) return nullptr;
  return (TypedValue*)&a->getValueRef(index);
}

TypedValue* SharedMap::NvGetStr(const ArrayData* ad, const StringData* key) {
  auto a = asSharedMap(ad);
  auto index = a->getIndex(key);
  if (index == -1) return nullptr;
  return (TypedValue*)&a->getValueRef(index);
}

void SharedMap::NvGetKey(const ArrayData* ad, TypedValue* out, ssize_t pos) {
  auto a = asSharedMap(ad);
  Variant k = a->m_arr->getKey(pos);
  TypedValue* tv = k.asTypedValue();
  // copy w/out clobbering out->_count.
  out->m_type = tv->m_type;
  out->m_data.num = tv->m_data.num;
  if (tv->m_type != KindOfInt64) out->m_data.pstr->incRefCount();
}

ArrayData* SharedMap::EscalateForSort(ArrayData* ad) {
  auto a = asSharedMap(ad);
  auto ret = a->m_arr->loadElems(*a, true /* mapInit */);
  assert(!ret->isStatic());
  return ret;
}

ssize_t SharedMap::IterBegin(const ArrayData* ad) {
  auto a = asSharedMap(ad);
  return a->m_size > 0 ? 0 : invalid_index;
}

ssize_t SharedMap::IterEnd(const ArrayData* ad) {
  auto a = asSharedMap(ad);
  auto n = a->m_size;
  return n > 0 ? ssize_t(n - 1) : invalid_index;
}

ssize_t SharedMap::IterAdvance(const ArrayData* ad, ssize_t prev) {
  auto a = asSharedMap(ad);
  assert(prev >= 0 && prev < a->m_size);
  ssize_t next = prev + 1;
  return next < a->m_size ? next : invalid_index;
}

ssize_t SharedMap::IterRewind(const ArrayData* ad, ssize_t prev) {
  assert(prev >= 0 && prev < asSharedMap(ad)->m_size);
  ssize_t next = prev - 1;
  return next >= 0 ? next : invalid_index;
}

bool SharedMap::ValidFullPos(const ArrayData* ad, const FullPos& fp) {
  assert(fp.getContainer() == ad);
  return false;
}

bool SharedMap::AdvanceFullPos(ArrayData* ad, FullPos& fp) {
  return false;
}

void SharedMap::getChildren(std::vector<TypedValue *> &out) {
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
