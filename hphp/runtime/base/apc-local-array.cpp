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
#include "hphp/runtime/base/member-val.h"
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
  assert(ad->isApcArray());
  assert(ad->checkCount());
  DEBUG_ONLY auto const local = static_cast<const APCLocalArray*>(ad);
  DEBUG_ONLY auto p = local->localCache();
  for (auto end = p + local->getSize(); p < end; ++p) {
    // Elements in the local cache must not be KindOfRef.
    assert(cellIsPlausible(*p));
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

member_rval::ptr_u APCLocalArray::GetValueRef(const ArrayData* adIn,
                                              ssize_t pos) {
  auto const ad = asApcArray(adIn);
  assert(unsigned(pos) < ad->getSize());
  auto const elms = ad->localCache();
  auto const tv = &elms[pos];
  if (tv->m_type != KindOfUninit) return tv;
  auto const sv = ad->m_arr->getValue(pos);
  tvAsVariant(tv) = sv->toLocal();
  assert(tv->m_type != KindOfUninit);
  return tv;
}

void APCLocalArray::Release(ArrayData* ad) {
  assert(ad->hasExactlyOneRef());
  auto const a = asApcArray(ad);
  auto size = a->heapSize();

  for (auto tv = a->localCache(), end = tv + a->m_size; tv < end; ++tv) {
    tvDecRefGen(tv);
  }
  a->m_arr->unreference();
  MM().removeApcArray(a);
  MM().objFree(a, size);
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
      ai.append(RvalAtPos(this, i).tv());
    }
    elems = ai.create();
  } else {
    ArrayInit ai(count, ArrayInit::Mixed{});
    for (uint32_t i = 0; i < count; i++) {
      ai.add(getKey(i), RvalAtPos(this, i).tv(), true);
    }
    elems = ai.create();
  }
  if (elems->isStatic()) {
    elems = elems->copy();
  }
  assert(elems->hasExactlyOneRef());
  return elems;
}

member_lval APCLocalArray::LvalInt(ArrayData* ad, int64_t k, bool /*copy*/) {
  EscalateHelper helper{ad};
  auto const lval = helper.escalated->lval(k, false);
  return member_lval { helper.release(lval.arr_base()), lval.elem() };
}

member_lval APCLocalArray::LvalIntRef(ArrayData* ad, int64_t k, bool /*copy*/) {
  EscalateHelper helper{ad};
  auto const lval = helper.escalated->lvalRef(k, false);
  return member_lval { helper.release(lval.arr_base()), lval.elem() };
}

member_lval
APCLocalArray::LvalStr(ArrayData* ad, StringData* k, bool /*copy*/) {
  EscalateHelper helper{ad};
  auto const lval = helper.escalated->lval(k, false);
  return member_lval { helper.release(lval.arr_base()), lval.elem() };
}

member_lval
APCLocalArray::LvalStrRef(ArrayData* ad, StringData* k, bool /*copy*/) {
  EscalateHelper helper{ad};
  auto const lval = helper.escalated->lvalRef(k, false);
  return member_lval { helper.release(lval.arr_base()), lval.elem() };
}

member_lval APCLocalArray::LvalNew(ArrayData* ad, bool /*copy*/) {
  EscalateHelper helper{ad};
  auto const lval = helper.escalated->lvalNew(false);
  return member_lval { helper.release(lval.arr_base()), lval.elem() };
}

member_lval APCLocalArray::LvalNewRef(ArrayData* ad, bool /*copy*/) {
  EscalateHelper helper{ad};
  auto const lval = helper.escalated->lvalNewRef(false);
  return member_lval { helper.release(lval.arr_base()), lval.elem() };
}

ArrayData*
APCLocalArray::SetInt(ArrayData* ad, int64_t k, Cell v, bool /*copy*/) {
  EscalateHelper helper{ad};
  return helper.release(helper.escalated->set(k, tvAsCVarRef(&v), false));
}

ArrayData*
APCLocalArray::SetStr(ArrayData* ad, StringData* k, Cell v, bool /*copy*/) {
  EscalateHelper helper{ad};
  return helper.release(helper.escalated->set(k, tvAsCVarRef(&v), false));
}

ArrayData*
APCLocalArray::SetRefInt(ArrayData* ad, int64_t k, Variant& v, bool /*copy*/) {
  EscalateHelper helper{ad};
  return helper.release(helper.escalated->setRef(k, v, false));
}

ArrayData* APCLocalArray::SetRefStr(ArrayData* ad, StringData* k, Variant& v,
                                    bool /*copy*/) {
  EscalateHelper helper{ad};
  return helper.release(helper.escalated->setRef(k, v, false));
}

ArrayData* APCLocalArray::RemoveInt(ArrayData* ad, int64_t k, bool /*copy*/) {
  EscalateHelper helper{ad};
  return helper.release(helper.escalated->remove(k, false));
}

ArrayData*
APCLocalArray::RemoveStr(ArrayData* ad, const StringData* k, bool /*copy*/) {
  EscalateHelper helper{ad};
  return helper.release(helper.escalated->remove(k, false));
}

ArrayData* APCLocalArray::Copy(const ArrayData* ad) {
  return Escalate(ad);
}

ArrayData* APCLocalArray::Append(ArrayData* ad, Cell v, bool /*copy*/) {
  EscalateHelper helper{ad};
  return helper.release(helper.escalated->append(v, false));
}

ArrayData* APCLocalArray::AppendRef(ArrayData* ad, Variant& v, bool /*copy*/) {
  EscalateHelper helper{ad};
  return helper.release(helper.escalated->appendRef(v, false));
}

ArrayData*
APCLocalArray::AppendWithRef(ArrayData* ad, TypedValue v, bool /*copy*/) {
  EscalateHelper helper{ad};
  return helper.release(helper.escalated->appendWithRef(v, false));
}

ArrayData* APCLocalArray::PlusEq(ArrayData* ad, const ArrayData *elems) {
  if (!elems->isPHPArray()) throwInvalidAdditionException(elems);
  auto escalated = Array::attach(Escalate(ad));
  return (escalated += const_cast<ArrayData*>(elems)).detach();
}

ArrayData* APCLocalArray::Merge(ArrayData* ad, const ArrayData *elems) {
  auto escalated = Array::attach(Escalate(ad));
  return escalated->merge(elems);
}

ArrayData* APCLocalArray::Prepend(ArrayData* ad, Cell v, bool /*copy*/) {
  EscalateHelper helper{ad};
  return helper.release(helper.escalated->prepend(v, false));
}

ArrayData *APCLocalArray::Escalate(const ArrayData* ad) {
  auto smap = asApcArray(ad);
  auto ret = smap->loadElems();
  assert(!ret->isStatic());
  assert(ret->hasExactlyOneRef());
  return ret;
}

member_rval::ptr_u APCLocalArray::NvGetInt(const ArrayData* ad, int64_t k) {
  auto a = asApcArray(ad);
  auto index = a->getIndex(k);
  if (index == -1) return nullptr;
  return GetValueRef(a, index);
}

member_rval::ptr_u APCLocalArray::NvGetStr(const ArrayData* ad,
                                           const StringData* key) {
  auto a = asApcArray(ad);
  auto index = a->getIndex(key);
  if (index == -1) return nullptr;
  return GetValueRef(a, index);
}

Cell APCLocalArray::NvGetKey(const ArrayData* ad, ssize_t pos) {
  auto a = asApcArray(ad);
  Variant k = a->m_arr->getKey(pos);
  auto const tv = k.asTypedValue();
  tvIncRefGen(tv);
  return *tv;
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
  assert(prev >= 0 && prev < a->m_size);
  ssize_t next = prev - 1;
  return next >= 0 ? next : a->m_size;
}

bool APCLocalArray::ValidMArrayIter(const ArrayData* ad,
                                    const MArrayIter& fp) {
  assert(fp.getContainer() == ad);
  not_reached();  // we should've escalated
}

bool APCLocalArray::AdvanceMArrayIter(ArrayData* /*ad*/, MArrayIter& /*fp*/) {
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
