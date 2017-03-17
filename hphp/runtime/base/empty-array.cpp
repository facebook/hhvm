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

#include "hphp/runtime/base/empty-array.h"

#include <type_traits>

#include "hphp/util/assertions.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/tv-helpers.h"
#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/packed-array-defs.h"
#include "hphp/runtime/base/set-array.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

std::aligned_storage<sizeof(ArrayData), 16>::type s_theEmptyArray;

struct EmptyArray::Initializer {
  Initializer() {
    auto const ad   = reinterpret_cast<ArrayData*>(&s_theEmptyArray);
    ad->m_sizeAndPos = 0;
    ad->initHeader(HeaderKind::Empty, StaticValue);
  }
};
EmptyArray::Initializer EmptyArray::s_initializer;

//////////////////////////////////////////////////////////////////////

void EmptyArray::Release(ArrayData*) {
  always_assert(!"never try to free the empty array");
}

Cell EmptyArray::NvGetKey(const ArrayData*, ssize_t pos) {
  // We have no valid positions---no one should call this function.
  not_reached();
}

size_t EmptyArray::Vsize(const ArrayData*) { not_reached(); }

const Variant& EmptyArray::GetValueRef(const ArrayData* ad, ssize_t pos) {
  // We have no valid positions---no one should call this function.
  not_reached();
}

// EmptyArray::IterAdvance() is reachable; see ArrayData::next() for details
ssize_t EmptyArray::IterAdvance(const ArrayData*, ssize_t prev) {
  return 0;
}

// EmptyArray::IterRewind() is NOT reachable; see ArrayData::prev() for details
ssize_t EmptyArray::IterRewind(const ArrayData*, ssize_t prev) {
  not_reached();
}

// Even though we always return false in ValidMArrayIter, this function may
// still be called because MArrayIters are constructed in an invalid position,
// and then advanced to the first element.
bool EmptyArray::AdvanceMArrayIter(ArrayData*, MArrayIter& fp) {
  return false;
}

// We're always already a static array.
void EmptyArray::OnSetEvalScalar(ArrayData*) { not_reached(); }
ArrayData* EmptyArray::CopyStatic(const ArrayData* ad) { not_reached(); }

//////////////////////////////////////////////////////////////////////

NEVER_INLINE
ArrayData* EmptyArray::Copy(const ArrayData*) { return staticEmptyArray(); }

ArrayData* EmptyArray::CopyWithStrongIterators(const ArrayData* ad) {
  // We can never have associated strong iterators, so we don't need
  // to do anything extra.
  return Copy(ad);
}

//////////////////////////////////////////////////////////////////////

/*
 * Note: if you try to tail-call these helper routines, gcc will
 * unfortunately still generate functions with frames and and makes a
 * call instead of a jump.  It's because of std::pair (and is still
 * the case if you return a custom struct).
 *
 * For now we're leaving this, because it's essentially free for these
 * routines to leave the lval pointer in the second return register,
 * and it seems questionable to clone the whole function just to avoid
 * the frame creation in these callers.  (It works to reinterpret_cast
 * these functions to one that returns ArrayData* instead of a pair in
 * the cases we don't need the second value, but this seems a tad too
 * sketchy for probably-unmeasurable benefits.  I'll admit I didn't
 * try to measure it though... ;)
 */

/*
 * Helper for empty array -> packed transitions.  Creates an array
 * with one element.  The element is transferred into the array (should
 * already be incref'd).
 */
ALWAYS_INLINE
ArrayLval EmptyArray::MakePackedInl(TypedValue tv) {
  auto const cap = kPackedSmallSize;
  auto const ad = static_cast<ArrayData*>(
    MM().objMalloc(sizeof(ArrayData) + cap * sizeof(TypedValue))
  );
  assert(cap == CapCode::ceil(cap).code);
  ad->m_sizeAndPos = 1; // size=1, pos=0
  ad->initHeader(CapCode::exact(cap), HeaderKind::Packed, 1);

  auto const lval = reinterpret_cast<TypedValue*>(ad + 1);
  lval->m_data = tv.m_data;
  lval->m_type = tv.m_type;

  assert(ad->kind() == ArrayData::kPackedKind);
  assert(ad->m_size == 1);
  assert(ad->m_pos == 0);
  assert(ad->hasExactlyOneRef());
  assert(PackedArray::checkInvariants(ad));
  return { ad, &tvAsVariant(lval) };
}

NEVER_INLINE
ArrayLval EmptyArray::MakePacked(TypedValue tv) {
  return MakePackedInl(tv);
}

/*
 * Helper for creating a single-element mixed array with a string key.
 *
 * Note: the key is not already incref'd, but the value must be.
 */
NEVER_INLINE
ArrayLval EmptyArray::MakeMixed(StringData* key, TypedValue val) {
  auto const ad = reqAllocArray(MixedArray::SmallScale);
  MixedArray::InitSmall(ad, 1/*count*/, 1/*size*/, 0/*nextIntKey*/);
  auto const data = ad->data();
  auto const hash = reinterpret_cast<int32_t*>(data + MixedArray::SmallSize);
  auto const khash = key->hash();
  auto const mask = MixedArray::SmallMask;
  hash[khash & mask] = 0;
  data[0].setStrKey(key, khash);

  auto& lval  = data[0].data;
  lval.m_data = val.m_data;
  lval.m_type = val.m_type;

  assert(ad->m_size == 1);
  assert(ad->m_pos == 0);
  assert(ad->m_scale == MixedArray::SmallScale);
  assert(ad->kind() == ArrayData::kMixedKind);
  assert(ad->hasExactlyOneRef());
  assert(ad->m_used == 1);
  assert(ad->checkInvariants());
  return { ad, &tvAsVariant(&lval) };
}

/*
 * Creating a single-element mixed array with a integer key.  The
 * value is already incref'd.
 */
ArrayLval EmptyArray::MakeMixed(int64_t key, TypedValue val) {
  auto const ad = reqAllocArray(MixedArray::SmallScale);
  MixedArray::InitSmall(ad, 1/*count*/, 1/*size*/, (key >= 0) ? key + 1 : 0);
  auto const data = ad->data();
  auto const hash = reinterpret_cast<int32_t*>(data + MixedArray::SmallSize);

  auto const mask = MixedArray::SmallMask;
  auto h = hash_int64(key);
  hash[h & mask] = 0;
  data[0].setIntKey(key, h);

  auto& lval  = data[0].data;
  lval.m_data = val.m_data;
  lval.m_type = val.m_type;

  assert(ad->kind() == ArrayData::kMixedKind);
  assert(ad->m_size == 1);
  assert(ad->m_pos == 0);
  assert(ad->hasExactlyOneRef());
  assert(ad->m_scale == MixedArray::SmallScale);
  assert(ad->m_used == 1);
  assert(ad->checkInvariants());
  return { ad, &tvAsVariant(&lval) };
}

//////////////////////////////////////////////////////////////////////

ArrayData* EmptyArray::SetInt(ArrayData*, int64_t k, Cell c, bool) {
  // TODO(#3888164): we should make it so we don't need KindOfUninit checks
  if (c.m_type == KindOfUninit) c.m_type = KindOfNull;
  tvRefcountedIncRef(&c);
  auto const ret = k == 0 ? EmptyArray::MakePacked(c)
                          : EmptyArray::MakeMixed(k, c);
  return ret.array;
}

ArrayData* EmptyArray::SetStr(ArrayData*,
                              StringData* k,
                              Cell val,
                              bool copy) {
  tvRefcountedIncRef(&val);
  // TODO(#3888164): we should make it so we don't need KindOfUninit checks
  if (val.m_type == KindOfUninit) val.m_type = KindOfNull;
  return EmptyArray::MakeMixed(k, val).array;
}

ArrayLval EmptyArray::LvalInt(ArrayData*, int64_t k, bool) {
  return k == 0 ? EmptyArray::MakePacked(make_tv<KindOfNull>())
                : EmptyArray::MakeMixed(k, make_tv<KindOfNull>());
}

ArrayLval EmptyArray::LvalIntRef(ArrayData* ad, int64_t k, bool copy) {
  if (RuntimeOption::EvalHackArrCompatNotices) raiseHackArrCompatRefBind(k);
  return LvalInt(ad, k, copy);
}

ArrayLval EmptyArray::LvalStr(ArrayData*, StringData* k, bool) {
  return EmptyArray::MakeMixed(k, make_tv<KindOfNull>());
}

ArrayLval EmptyArray::LvalStrRef(ArrayData* ad, StringData* k, bool copy) {
  if (RuntimeOption::EvalHackArrCompatNotices) raiseHackArrCompatRefBind(k);
  return LvalStr(ad, k, copy);
}

ArrayLval EmptyArray::LvalNew(ArrayData*, bool) {
  return EmptyArray::MakePacked(make_tv<KindOfNull>());
}

ArrayLval EmptyArray::LvalNewRef(ArrayData* ad, bool copy) {
  if (RuntimeOption::EvalHackArrCompatNotices) raiseHackArrCompatRefNew();
  return LvalNew(ad, copy);
}

ArrayData* EmptyArray::SetRefInt(ArrayData*,
                                 int64_t k,
                                 Variant& var,
                                 bool) {
  if (RuntimeOption::EvalHackArrCompatNotices) raiseHackArrCompatRefBind(k);
  auto ref = *var.asRef();
  tvIncRef(&ref);
  auto const ret = k == 0 ? EmptyArray::MakePacked(ref)
                          : EmptyArray::MakeMixed(k, ref);
  return ret.array;
}

ArrayData* EmptyArray::SetRefStr(ArrayData*,
                                 StringData* k,
                                 Variant& var,
                                 bool) {
  if (RuntimeOption::EvalHackArrCompatNotices) raiseHackArrCompatRefBind(k);
  auto ref = *var.asRef();
  tvIncRef(&ref);
  return EmptyArray::MakeMixed(k, ref).array;
}

ArrayData* EmptyArray::Append(ArrayData*, Cell v, bool copy) {
  tvRefcountedIncRef(&v);
  return EmptyArray::MakePackedInl(v).array;
}

ArrayData* EmptyArray::AppendRef(ArrayData*, Variant& v, bool copy) {
  if (RuntimeOption::EvalHackArrCompatNotices) raiseHackArrCompatRefNew();
  auto ref = *v.asRef();
  tvIncRef(&ref);
  return EmptyArray::MakePacked(ref).array;
}

ArrayData* EmptyArray::AppendWithRef(ArrayData*, const Variant& v, bool copy) {
  if (RuntimeOption::EvalHackArrCompatNotices && v.isReferenced()) {
    raiseHackArrCompatRefNew();
  }
  auto tv = make_tv<KindOfNull>();
  tvAsVariant(&tv).setWithRef(v);
  return EmptyArray::MakePacked(tv).array;
}

//////////////////////////////////////////////////////////////////////

ArrayData* EmptyArray::PlusEq(ArrayData*, const ArrayData* elems) {
  if (!elems->isPHPArray()) throwInvalidAdditionException(elems);
  elems->incRefCount();
  return const_cast<ArrayData*>(elems);
}

ArrayData* EmptyArray::Merge(ArrayData*, const ArrayData* elems) {
  // Packed arrays don't need renumbering, so don't make a copy.
  if (elems->isPacked()) {
    elems->incRefCount();
    return const_cast<ArrayData*>(elems);
  }
  // Fast path the common case that elems is mixed.
  if (elems->isMixed()) {
    auto const copy = MixedArray::Copy(elems);
    assert(copy != elems);
    MixedArray::Renumber(copy);
    return copy;
  }
  auto copy = const_cast<ArrayData*>(elems)->toPHPArray(true);
  copy = copy == elems ? elems->copy() : copy;
  assert(copy != elems);
  copy->renumber();
  return copy;
}

ArrayData* EmptyArray::PopOrDequeue(ArrayData* ad, Variant& value) {
  value = uninit_null();
  return ad;
}

ArrayData* EmptyArray::Prepend(ArrayData*, Cell v, bool) {
  tvRefcountedIncRef(&v);
  return EmptyArray::MakePacked(v).array;
}

ArrayData* EmptyArray::ToDict(ArrayData*, bool) {
  return staticEmptyDictArray();
}

ArrayData* EmptyArray::ToVec(ArrayData*, bool) {
  return staticEmptyVecArray();
}

ArrayData* EmptyArray::ToKeyset(ArrayData*, bool) {
  return staticEmptyKeysetArray();
}

//////////////////////////////////////////////////////////////////////

ArrayData* EmptyArray::ZSetInt(ArrayData* ad, int64_t k, RefData* v) {
  auto const arr = MixedArray::MakeReserveMixed(MixedArray::SmallSize);
  DEBUG_ONLY auto const tmp = arr->zSet(k, v);
  assert(tmp == arr);
  return arr;
}

ArrayData* EmptyArray::ZSetStr(ArrayData* ad, StringData* k, RefData* v) {
  auto const arr = MixedArray::MakeReserveMixed(MixedArray::SmallSize);
  DEBUG_ONLY auto const tmp = arr->zSet(k, v);
  assert(tmp == arr);
  return arr;
}

ArrayData* EmptyArray::ZAppend(ArrayData* ad, RefData* v, int64_t* key_ptr) {
  auto const arr = MixedArray::MakeReserveMixed(MixedArray::SmallSize);
  DEBUG_ONLY auto const tmp = arr->zAppend(v, key_ptr);
  assert(tmp == arr);
  return arr;
}

//////////////////////////////////////////////////////////////////////

}
