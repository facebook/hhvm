/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/base/apc-local-array.h"

#include <vector>

#include "hphp/runtime/base/apc-handle-defs.h"
#include "hphp/runtime/base/apc-typed-value.h"
#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/hphp-array-defs.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/runtime-error.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

const Variant& APCLocalArray::getValueRef(ssize_t pos) const {
  APCHandle *sv = m_arr->getValue(pos);
  DataType t = sv->getType();
  if (!IS_REFCOUNTED_TYPE(t)) {
    return APCTypedValue::fromHandle(sv)->asCVarRef();
  }
  if (LIKELY(m_localCache != nullptr)) {
    assert(unsigned(pos) < m_arr->capacity());
    TypedValue* tv = &m_localCache[pos];
    if (tv->m_type != KindOfUninit) {
      return tvAsCVarRef(tv);
    }
  } else {
    static_assert(KindOfUninit == 0, "must be 0 since we use smart_calloc");
    unsigned cap = m_arr->capacity();
    m_localCache = (TypedValue*) smart_calloc(cap, sizeof(TypedValue));
  }
  TypedValue* tv = &m_localCache[pos];
  tvAsVariant(tv) = sv->toLocal();
  assert(tv->m_type != KindOfUninit);
  return tvAsCVarRef(tv);
}

ALWAYS_INLINE APCLocalArray::~APCLocalArray() {
  if (m_localCache) {
    for (TypedValue* tv = m_localCache, *end = tv + m_arr->capacity();
         tv < end; ++tv) {
      tvRefcountedDecRef(tv);
    }
    smart_free(m_localCache);
  }
  m_arr->getHandle()->unreference();
}

void APCLocalArray::Release(ArrayData* ad) {
  auto const smap = asSharedArray(ad);
  smap->~APCLocalArray();
  MM().smartFreeSize(smap, sizeof(APCLocalArray));
}

bool APCLocalArray::IsVectorData(const ArrayData* ad) {
  auto a = asSharedArray(ad);
  const auto n = a->size();
  for (ssize_t i = 0; i < n; i++) {
    if (a->getIndex(i) != i) return false;
  }
  return true;
}

bool APCLocalArray::ExistsStr(const ArrayData* ad, const StringData* k) {
  auto a = asSharedArray(ad);
  return a->getIndex(k) != -1;
}

bool APCLocalArray::ExistsInt(const ArrayData* ad, int64_t k) {
  return asSharedArray(ad)->getIndex(k) != -1;
}

ssize_t APCLocalArray::getIndex(int64_t k) const {
  return m_arr->getIndex(k);
}

ssize_t APCLocalArray::getIndex(const StringData* k) const {
  return m_arr->getIndex(k);
}

ArrayData* APCLocalArray::loadElems() const {
  auto count = m_arr->size();
  ArrayData* elems;
  if (m_arr->isPacked()) {
    PackedArrayInit ai(count);
    for (uint i = 0; i < count; i++) {
      ai.append(getValueRef(i));
    }
    elems = ai.create();
  } else {
    ArrayInit ai(count);
    for (uint i = 0; i < count; i++) {
      ai.add(getKey(i), getValueRef(i), true);
    }
    elems = ai.create();
  }
  if (elems->isStatic()) {
    elems = elems->copy();
  }
  return elems;
}

/* if a2 is modified copy of a1 (i.e. != a1), then release a1 and return a2 */
static inline ArrayData* releaseIfCopied(ArrayData* a1, ArrayData* a2) {
  if (a1 != a2) a1->release();
  return a2;
}

ArrayData *APCLocalArray::LvalInt(ArrayData* ad, int64_t k, Variant *&ret,
                                  bool copy) {
  ArrayData *escalated = Escalate(ad);
  return releaseIfCopied(escalated, escalated->lval(k, ret, false));
}

ArrayData *APCLocalArray::LvalStr(ArrayData* ad, StringData* k, Variant *&ret,
                                  bool copy) {
  ArrayData *escalated = Escalate(ad);
  return releaseIfCopied(escalated, escalated->lval(k, ret, false));
}

ArrayData *APCLocalArray::LvalNew(ArrayData* ad, Variant *&ret, bool copy) {
  ArrayData *escalated = Escalate(ad);
  return releaseIfCopied(escalated, escalated->lvalNew(ret, false));
}

ArrayData*
APCLocalArray::SetInt(ArrayData* ad, int64_t k, const Variant& v, bool copy) {
  ArrayData *escalated = Escalate(ad);
  return releaseIfCopied(escalated, escalated->set(k, v, false));
}

ArrayData*
APCLocalArray::SetStr(ArrayData* ad, StringData* k, const Variant& v, bool copy) {
  ArrayData *escalated = Escalate(ad);
  return releaseIfCopied(escalated, escalated->set(k, v, false));
}

ArrayData*
APCLocalArray::SetRefInt(ArrayData* ad, int64_t k, const Variant& v, bool copy) {
  ArrayData *escalated = Escalate(ad);
  return releaseIfCopied(escalated, escalated->setRef(k, v, false));
}

ArrayData*
APCLocalArray::SetRefStr(ArrayData* ad, StringData* k, const Variant& v, bool copy) {
  ArrayData *escalated = Escalate(ad);
  return releaseIfCopied(escalated, escalated->setRef(k, v, false));
}

ArrayData *APCLocalArray::RemoveInt(ArrayData* ad, int64_t k, bool copy) {
  ArrayData *escalated = Escalate(ad);
  return releaseIfCopied(escalated, escalated->remove(k, false));
}

ArrayData*
APCLocalArray::RemoveStr(ArrayData* ad, const StringData* k, bool copy) {
  ArrayData *escalated = Escalate(ad);
  return releaseIfCopied(escalated, escalated->remove(k, false));
}

ArrayData* APCLocalArray::Copy(const ArrayData* ad) {
  return Escalate(ad);
}

ArrayData *APCLocalArray::Append(ArrayData* ad, const Variant& v, bool copy) {
  ArrayData *escalated = Escalate(ad);
  return releaseIfCopied(escalated, escalated->append(v, false));
}

ArrayData*
APCLocalArray::AppendRef(ArrayData* ad, const Variant& v, bool copy) {
  ArrayData *escalated = Escalate(ad);
  return releaseIfCopied(escalated, escalated->appendRef(v, false));
}

ArrayData*
APCLocalArray::AppendWithRef(ArrayData* ad, const Variant& v, bool copy) {
  ArrayData *escalated = Escalate(ad);
  return releaseIfCopied(escalated, escalated->appendWithRef(v, false));
}

ArrayData* APCLocalArray::PlusEq(ArrayData* ad, const ArrayData *elems) {
  Array escalated = Escalate(ad);
  return (escalated += const_cast<ArrayData*>(elems)).detach();
}

ArrayData* APCLocalArray::Merge(ArrayData* ad, const ArrayData *elems) {
  Array escalated = Escalate(ad);
  return escalated->merge(elems);
}

ArrayData *APCLocalArray::Prepend(ArrayData* ad, const Variant& v, bool copy) {
  ArrayData *escalated = Escalate(ad);
  return releaseIfCopied(escalated, escalated->prepend(v, false));
}

ArrayData *APCLocalArray::Escalate(const ArrayData* ad) {
  auto smap = asSharedArray(ad);
  auto ret = smap->loadElems();
  assert(!ret->isStatic());
  return ret;
}

TypedValue* APCLocalArray::NvGetInt(const ArrayData* ad, int64_t k) {
  auto a = asSharedArray(ad);
  auto index = a->getIndex(k);
  if (index == -1) return nullptr;
  return (TypedValue*)&a->getValueRef(index);
}

TypedValue* APCLocalArray::NvGetStr(const ArrayData* ad,
                                    const StringData* key) {
  auto a = asSharedArray(ad);
  auto index = a->getIndex(key);
  if (index == -1) return nullptr;
  return (TypedValue*)&a->getValueRef(index);
}

void APCLocalArray::NvGetKey(const ArrayData* ad,
                             TypedValue* out,
                             ssize_t pos) {
  auto a = asSharedArray(ad);
  Variant k = a->m_arr->getKey(pos);
  TypedValue* tv = k.asTypedValue();
  // copy w/out clobbering out->_count.
  out->m_type = tv->m_type;
  out->m_data.num = tv->m_data.num;
  if (tv->m_type != KindOfInt64) out->m_data.pstr->incRefCount();
}

ArrayData* APCLocalArray::EscalateForSort(ArrayData* ad) {
  auto a = asSharedArray(ad);
  auto ret = a->loadElems();
  assert(!ret->isStatic());
  return ret;
}

ssize_t APCLocalArray::IterBegin(const ArrayData* ad) {
  auto a = asSharedArray(ad);
  return a->m_size > 0 ? 0 : invalid_index;
}

ssize_t APCLocalArray::IterEnd(const ArrayData* ad) {
  auto a = asSharedArray(ad);
  auto n = a->m_size;
  return n > 0 ? ssize_t(n - 1) : invalid_index;
}

ssize_t APCLocalArray::IterAdvance(const ArrayData* ad, ssize_t prev) {
  auto a = asSharedArray(ad);
  return a->iterAdvanceImpl(prev);
}

ssize_t APCLocalArray::IterRewind(const ArrayData* ad, ssize_t prev) {
  assert(prev >= 0 && prev < asSharedArray(ad)->m_size);
  ssize_t next = prev - 1;
  return next >= 0 ? next : invalid_index;
}

bool APCLocalArray::ValidMArrayIter(const ArrayData* ad, const MArrayIter& fp) {
  assert(fp.getContainer() == ad);
  return false;
}

bool APCLocalArray::AdvanceMArrayIter(ArrayData* ad, MArrayIter& fp) {
  return false;
}

void APCLocalArray::getChildren(std::vector<TypedValue *> &out) {
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
