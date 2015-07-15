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

#include "hphp/runtime/base/apc-local-array-defs.h"
#include "hphp/runtime/base/apc-handle-defs.h"
#include "hphp/runtime/base/apc-typed-value.h"
#include "hphp/runtime/base/array-data-defs.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/type-conversions.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

bool APCLocalArray::checkInvariants(const ArrayData* ad) {
  assert(ad->isApcArray());
  assert(ad->getCount() != 0);
  DEBUG_ONLY auto const shared = static_cast<const APCLocalArray*>(ad);
  if (auto ptr = shared->m_localCache) {
    auto const cap = shared->m_arr->capacity();
    auto const stop = ptr + cap;
    for (; ptr != stop; ++ptr) {
      // Elements in the local cache must not be KindOfRef.
      assert(cellIsPlausible(*ptr));
    }
  }
  return true;
}

ALWAYS_INLINE
Variant APCLocalArray::getKey(ssize_t pos) const {
  return m_arr->getKey(pos);
}

void APCLocalArray::sweep() {
  m_arr->getHandle()->unreference();
}

const Variant& APCLocalArray::GetValueRef(const ArrayData* adIn, ssize_t pos) {
  auto const ad = asApcArray(adIn);
  auto const sv = ad->m_arr->getValue(pos);
  if (LIKELY(ad->m_localCache != nullptr)) {
    assert(unsigned(pos) < ad->m_arr->capacity());
    TypedValue* tv = &ad->m_localCache[pos];
    if (tv->m_type != KindOfUninit) {
      return tvAsCVarRef(tv);
    }
  } else {
    static_assert(KindOfUninit == 0, "must be 0 since we use req::calloc");
    unsigned cap = ad->m_arr->capacity();
    ad->m_localCache = static_cast<TypedValue*>(
      req::calloc(cap, sizeof(TypedValue))
    );
  }
  auto const tv = &ad->m_localCache[pos];
  tvAsVariant(tv) = sv->toLocal();
  assert(tv->m_type != KindOfUninit);
  return tvAsCVarRef(tv);
}

ALWAYS_INLINE
APCLocalArray::~APCLocalArray() {
  if (m_localCache) {
    for (TypedValue* tv = m_localCache, *end = tv + m_arr->capacity();
         tv < end; ++tv) {
      tvRefcountedDecRef(tv);
    }
    req::free(m_localCache);
  }
  m_arr->getHandle()->unreference();
  MM().removeApcArray(this);
}

void APCLocalArray::Release(ArrayData* ad) {
  assert(ad->hasExactlyOneRef());
  auto const a = asApcArray(ad);
  a->~APCLocalArray();
  MM().freeSmallSize(a, sizeof(APCLocalArray));
}

size_t APCLocalArray::Vsize(const ArrayData*) { not_reached(); }

bool APCLocalArray::IsVectorData(const ArrayData* ad) {
  auto a = asApcArray(ad);
  const auto n = a->size();
  for (ssize_t i = 0; i < n; i++) {
    if (a->getIndex(i) != i) return false;
  }
  return true;
}

bool APCLocalArray::ExistsStr(const ArrayData* ad, const StringData* k) {
  auto a = asApcArray(ad);
  return a->getIndex(k) != -1;
}

bool APCLocalArray::ExistsInt(const ArrayData* ad, int64_t k) {
  return asApcArray(ad)->getIndex(k) != -1;
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
    for (uint32_t i = 0; i < count; i++) {
      ai.append(GetValueRef(this, i));
    }
    elems = ai.create();
  } else {
    ArrayInit ai(count, ArrayInit::Mixed{});
    for (uint32_t i = 0; i < count; i++) {
      ai.add(getKey(i), GetValueRef(this, i), true);
    }
    elems = ai.create();
  }
  if (elems->isStatic()) {
    elems = elems->copy();
  }
  assert(elems->hasExactlyOneRef());
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
APCLocalArray::SetInt(ArrayData* ad, int64_t k, Cell v, bool copy) {
  ArrayData *escalated = Escalate(ad);
  return releaseIfCopied(escalated, escalated->set(k, tvAsCVarRef(&v), false));
}

ArrayData*
APCLocalArray::SetStr(ArrayData* ad, StringData* k, Cell v, bool copy) {
  ArrayData *escalated = Escalate(ad);
  return releaseIfCopied(escalated, escalated->set(k, tvAsCVarRef(&v), false));
}

ArrayData*
APCLocalArray::SetRefInt(ArrayData* ad, int64_t k, Variant& v, bool copy) {
  ArrayData *escalated = Escalate(ad);
  return releaseIfCopied(escalated, escalated->setRef(k, v, false));
}

ArrayData*
APCLocalArray::SetRefStr(ArrayData* ad, StringData* k, Variant& v, bool copy) {
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

ArrayData* APCLocalArray::CopyWithStrongIterators(const ArrayData*) {
  throw FatalErrorException(
    "Unimplemented ArrayData::copyWithStrongIterators");
}

ArrayData* APCLocalArray::Append(ArrayData* ad, const Variant& v, bool copy) {
  ArrayData* escalated = Escalate(ad);
  return releaseIfCopied(escalated, escalated->append(v, false));
}

ArrayData*
APCLocalArray::AppendRef(ArrayData* ad, Variant& v, bool copy) {
  ArrayData *escalated = Escalate(ad);
  return releaseIfCopied(escalated, escalated->appendRef(v, false));
}

ArrayData*
APCLocalArray::AppendWithRef(ArrayData* ad, const Variant& v, bool copy) {
  ArrayData *escalated = Escalate(ad);
  return releaseIfCopied(escalated, escalated->appendWithRef(v, false));
}

ArrayData* APCLocalArray::PlusEq(ArrayData* ad, const ArrayData *elems) {
  auto escalated = Array::attach(Escalate(ad));
  return (escalated += const_cast<ArrayData*>(elems)).detach();
}

ArrayData* APCLocalArray::Merge(ArrayData* ad, const ArrayData *elems) {
  auto escalated = Array::attach(Escalate(ad));
  return escalated->merge(elems);
}

ArrayData *APCLocalArray::Prepend(ArrayData* ad, const Variant& v, bool copy) {
  ArrayData *escalated = Escalate(ad);
  return releaseIfCopied(escalated, escalated->prepend(v, false));
}

ArrayData *APCLocalArray::Escalate(const ArrayData* ad) {
  auto smap = asApcArray(ad);
  auto ret = smap->loadElems();
  assert(!ret->isStatic());
  assert(ret->hasExactlyOneRef());
  return ret;
}

const TypedValue* APCLocalArray::NvGetInt(const ArrayData* ad, int64_t k) {
  auto a = asApcArray(ad);
  auto index = a->getIndex(k);
  if (index == -1) return nullptr;
  return GetValueRef(a, index).asTypedValue();
}

const TypedValue* APCLocalArray::NvGetStr(const ArrayData* ad,
                                    const StringData* key) {
  auto a = asApcArray(ad);
  auto index = a->getIndex(key);
  if (index == -1) return nullptr;
  return GetValueRef(a, index).asTypedValue();
}

void APCLocalArray::NvGetKey(const ArrayData* ad,
                             TypedValue* out,
                             ssize_t pos) {
  auto a = asApcArray(ad);
  Variant k = a->m_arr->getKey(pos);
  TypedValue* tv = k.asTypedValue();
  // copy w/out clobbering out->_count.
  out->m_type = tv->m_type;
  out->m_data.num = tv->m_data.num;
  if (tv->m_type != KindOfInt64) out->m_data.pstr->incRefCount();
}

ArrayData* APCLocalArray::EscalateForSort(ArrayData* ad, SortFunction sf) {
  auto a = asApcArray(ad);
  ArrayData* elems = a->loadElems();
  ArrayData* ret = elems->escalateForSort(sf);
  if (ret != elems) {
    elems->release();
  }
  assert(ret->hasExactlyOneRef());
  assert(!ret->isStatic());
  return ret;
}

void APCLocalArray::Ksort(ArrayData*, int sort_flags, bool ascending) {
  not_reached();
}

void APCLocalArray::Sort(ArrayData*, int sort_flags, bool ascending) {
  not_reached();
}

void APCLocalArray::Asort(ArrayData*, int sort_flags, bool ascending) {
  not_reached();
}

bool APCLocalArray::Uksort(ArrayData*, const Variant& cmp_function) {
  not_reached();
}

bool APCLocalArray::Usort(ArrayData*, const Variant& cmp_function) {
  not_reached();
}

bool APCLocalArray::Uasort(ArrayData*, const Variant& cmp_function) {
  not_reached();
}

ssize_t APCLocalArray::IterBegin(const ArrayData* ad) {
  return 0;
}

ssize_t APCLocalArray::IterLast(const ArrayData* ad) {
  auto a = asApcArray(ad);
  auto n = a->m_size;
  return n > 0 ? ssize_t(n - 1) : 0;
}

ssize_t APCLocalArray::IterEnd(const ArrayData* ad) {
  auto a = asApcArray(ad);
  auto n = a->m_size;
  return n;
}

ssize_t APCLocalArray::IterAdvance(const ArrayData* ad, ssize_t prev) {
  auto a = asApcArray(ad);
  return a->iterAdvanceImpl(prev);
}

ssize_t APCLocalArray::IterRewind(const ArrayData* ad, ssize_t prev) {
  auto a = asApcArray(ad);
  assert(prev >= 0 && prev < a->m_size);
  ssize_t next = prev - 1;
  return next >= 0 ? next : a->m_size;
}

bool APCLocalArray::ValidMArrayIter(const ArrayData* ad,
                                    const MArrayIter& fp) {
  assert(fp.getContainer() == ad);
  not_reached();  // we should've escalated
}

bool APCLocalArray::AdvanceMArrayIter(ArrayData* ad, MArrayIter& fp) {
  not_reached();  // we should've escalated
}

ArrayData* APCLocalArray::CopyStatic(const ArrayData*) {
  raise_error("APCLocalArray::copyStatic not implemented.");
  return nullptr;
}

void APCLocalArray::Renumber(ArrayData*) {
}

void APCLocalArray::OnSetEvalScalar(ArrayData*) {
  not_reached();
}

///////////////////////////////////////////////////////////////////////////////
}
