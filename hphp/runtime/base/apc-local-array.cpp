/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/base/apc-local-array-defs.h"
#include "hphp/runtime/base/apc-handle-defs.h"
#include "hphp/runtime/base/apc-typed-value.h"
#include "hphp/runtime/base/array-data-defs.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/tv-val.h"
#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/tv-refcount.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

namespace {

// Helper for when we need to escalate the array and then perform an operation
// on it which may then mutate it further. This handles optionally releasing the
// escalated intermediate array in an exception-safe way.
struct EscalateHelper {
  explicit EscalateHelper(const ArrayData* in)
    : escalated{APCLocalArray::Escalate(in)} {}

  ~EscalateHelper() {
    if (escalated) escalated->release();
  }

  // Release ownership of the escalated array. If it has mutated to a new array,
  // the original escalated array will be released upon destruction.
  ArrayData* release(ArrayData* in) {
    if (escalated == in) escalated = nullptr;
    return in;
  }

  ArrayData* escalated;
};

}

//////////////////////////////////////////////////////////////////////

bool APCLocalArray::checkInvariants(const ArrayData* ad) {
  assertx(ad->isApcArrayKind());
  assertx(ad->isNotDVArray());
  assertx(ad->checkCount());
  DEBUG_ONLY auto const local = static_cast<const APCLocalArray*>(ad);
  DEBUG_ONLY auto p = local->localCache();
  for (auto end = p + local->getSize(); p < end; ++p) {
    assertx(tvIsPlausible(*p));
  }
  return true;
}

ALWAYS_INLINE
Variant APCLocalArray::getKey(ssize_t pos) const {
  return m_arr->getKey(pos);
}

void APCLocalArray::sweep() {
  m_arr->unreference();
  m_arr = nullptr;
}

tv_rval APCLocalArray::RvalPos(const ArrayData* adIn, ssize_t pos) {
  auto const ad = asApcArray(adIn);
  assertx(unsigned(pos) < ad->getSize());
  auto const elms = ad->localCache();
  auto const tv = &elms[pos];
  if (tv->m_type != KindOfUninit) return tv;
  auto const sv = ad->m_arr->getValue(pos);
  tvAsVariant(tv) = sv->toLocal();
  assertx(tv->m_type != KindOfUninit);
  return tv;
}

void APCLocalArray::Release(ArrayData* ad) {
  ad->fixCountForRelease();
  assertx(ad->hasExactlyOneRef());
  auto const a = asApcArray(ad);
  auto size = a->heapSize();

  for (auto tv = a->localCache(), end = tv + a->m_size; tv < end; ++tv) {
    tvDecRefGen(tv);
  }
  a->m_arr->unreference();
  tl_heap->removeApcArray(a);
  tl_heap->objFree(a, size);
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
      ai.append(RvalPos(this, i).tv());
    }
    elems = ai.create();
  } else {
    ArrayInit ai(count, ArrayInit::Mixed{});
    for (uint32_t i = 0; i < count; i++) {
      ai.add(getKey(i), RvalPos(this, i).tv(), true);
    }
    elems = ai.create();
  }
  if (elems->isStatic()) {
    elems = elems->copy();
  }
  assertx(elems->hasExactlyOneRef());
  return elems;
}

arr_lval APCLocalArray::LvalInt(ArrayData* ad, int64_t k, bool /*copy*/) {
  EscalateHelper helper{ad};
  auto const lval = helper.escalated->lval(k, false);
  return arr_lval { helper.release(lval.arr), lval };
}

arr_lval
APCLocalArray::LvalStr(ArrayData* ad, StringData* k, bool /*copy*/) {
  EscalateHelper helper{ad};
  auto const lval = helper.escalated->lval(k, false);
  return arr_lval { helper.release(lval.arr), lval };
}

ArrayData* APCLocalArray::SetInt(ArrayData* ad, int64_t k, TypedValue v) {
  EscalateHelper helper{ad};
  return helper.release(helper.escalated->set(k, v));
}

ArrayData* APCLocalArray::SetIntMove(ArrayData* ad, int64_t k, TypedValue v) {
  EscalateHelper helper{ad};
  auto const result = helper.escalated->setMove(k, v);
  helper.escalated = nullptr;
  return result;
}

ArrayData* APCLocalArray::SetStr(ArrayData* ad, StringData* k, TypedValue v) {
  EscalateHelper helper{ad};
  return helper.release(helper.escalated->set(k, v));
}

ArrayData* APCLocalArray::SetStrMove(ArrayData* ad, StringData* k, TypedValue v) {
  EscalateHelper helper{ad};
  auto const result = helper.escalated->setMove(k, v);
  helper.escalated = nullptr;
  return result;
}

ArrayData* APCLocalArray::RemoveInt(ArrayData* ad, int64_t k) {
  EscalateHelper helper{ad};
  return helper.release(helper.escalated->remove(k));
}

ArrayData* APCLocalArray::RemoveStr(ArrayData* ad, const StringData* k) {
  EscalateHelper helper{ad};
  return helper.release(helper.escalated->remove(k));
}

ArrayData* APCLocalArray::Copy(const ArrayData* ad) {
  return Escalate(ad);
}

ArrayData* APCLocalArray::Append(ArrayData* ad, TypedValue v) {
  EscalateHelper helper{ad};
  return helper.release(helper.escalated->append(v));
}

ArrayData* APCLocalArray::PlusEq(ArrayData* ad, const ArrayData *elems) {
  if (!elems->isPHPArrayType()) throwInvalidAdditionException(elems);
  auto escalated = Array::attach(Escalate(ad));
  return (escalated += const_cast<ArrayData*>(elems)).detach();
}

ArrayData* APCLocalArray::Merge(ArrayData* ad, const ArrayData *elems) {
  auto escalated = Array::attach(Escalate(ad));
  return escalated->merge(elems);
}

ArrayData* APCLocalArray::Prepend(ArrayData* ad, TypedValue v) {
  EscalateHelper helper{ad};
  return helper.release(helper.escalated->prepend(v));
}

ArrayData *APCLocalArray::Escalate(const ArrayData* ad) {
  auto smap = asApcArray(ad);
  auto ret = smap->loadElems();
  assertx(!ret->isStatic());
  assertx(ret->hasExactlyOneRef());
  return ret;
}

tv_rval APCLocalArray::NvGetInt(const ArrayData* ad, int64_t k) {
  auto a = asApcArray(ad);
  auto index = a->getIndex(k);
  if (index == -1) return nullptr;
  return RvalPos(a, index);
}

tv_rval APCLocalArray::NvGetStr(const ArrayData* ad,
                                           const StringData* key) {
  auto a = asApcArray(ad);
  auto index = a->getIndex(key);
  if (index == -1) return nullptr;
  return RvalPos(a, index);
}

ssize_t APCLocalArray::NvGetIntPos(const ArrayData* ad, int64_t k) {
  auto a = asApcArray(ad);
  auto index = a->getIndex(k);
  return (index == -1) ? a->m_size : index;
}

ssize_t APCLocalArray::NvGetStrPos(const ArrayData* ad, const StringData* k) {
  auto a = asApcArray(ad);
  auto index = a->getIndex(k);
  return (index == -1) ? a->m_size : index;
}

TypedValue APCLocalArray::GetPosKey(const ArrayData* ad, ssize_t pos) {
  return *asApcArray(ad)->m_arr->getKey(pos).asTypedValue();
}

TypedValue APCLocalArray::GetPosVal(const ArrayData* ad, ssize_t pos) {
  return *RvalPos(ad, pos);
}

ArrayData* APCLocalArray::EscalateForSort(ArrayData* ad, SortFunction sf) {
  auto a = asApcArray(ad);
  ArrayData* elems = a->loadElems();
  ArrayData* ret = elems->escalateForSort(sf);
  if (ret != elems) {
    elems->release();
  }
  assertx(ret->empty() || ret->hasExactlyOneRef());
  return ret;
}

void APCLocalArray::Ksort(ArrayData*, int /*sort_flags*/, bool /*ascending*/) {
  not_reached();
}

void APCLocalArray::Sort(ArrayData*, int /*sort_flags*/, bool /*ascending*/) {
  not_reached();
}

void APCLocalArray::Asort(ArrayData*, int /*sort_flags*/, bool /*ascending*/) {
  not_reached();
}

bool APCLocalArray::Uksort(ArrayData*, const Variant& /*cmp_function*/) {
  not_reached();
}

bool APCLocalArray::Usort(ArrayData*, const Variant& /*cmp_function*/) {
  not_reached();
}

bool APCLocalArray::Uasort(ArrayData*, const Variant& /*cmp_function*/) {
  not_reached();
}

ssize_t APCLocalArray::IterBegin(const ArrayData* /*ad*/) {
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
  assertx(prev >= 0 && prev < a->m_size);
  ssize_t next = prev - 1;
  return next >= 0 ? next : a->m_size;
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
