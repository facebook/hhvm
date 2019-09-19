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

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/tv-val.h"
#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/packed-array-defs.h"
#include "hphp/runtime/base/set-array.h"
#include "hphp/runtime/base/tv-mutate.h"
#include "hphp/runtime/base/tv-type.h"
#include "hphp/runtime/base/tv-variant.h"
#include "hphp/runtime/base/type-variant.h"

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

Cell EmptyArray::NvGetKey(const ArrayData*, ssize_t /*pos*/) {
  // We have no valid positions---no one should call this function.
  not_reached();
}

size_t EmptyArray::Vsize(const ArrayData*) { not_reached(); }

tv_rval EmptyArray::GetValueRef(const ArrayData* /*ad*/, ssize_t /*pos*/) {
  // We have no valid positions---no one should call this function.
  not_reached();
}

// EmptyArray::IterAdvance() is reachable; see ArrayData::next() for details
ssize_t EmptyArray::IterAdvance(const ArrayData*, ssize_t /*prev*/) {
  return 0;
}

// EmptyArray::IterRewind() is NOT reachable; see ArrayData::prev() for details
ssize_t EmptyArray::IterRewind(const ArrayData*, ssize_t /*prev*/) {
  not_reached();
}

// We're always already a static array.
void EmptyArray::OnSetEvalScalar(ArrayData*) { not_reached(); }
ArrayData* EmptyArray::CopyStatic(const ArrayData* /*ad*/) {
  not_reached();
}

//////////////////////////////////////////////////////////////////////

NEVER_INLINE
ArrayData* EmptyArray::Copy(const ArrayData*) { return ArrayData::Create(); }

//////////////////////////////////////////////////////////////////////

/*
 * Note: if you try to tail-call these helper routines, gcc will
 * unfortunately still generate functions with frames and makes a
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
arr_lval EmptyArray::MakePackedInl(TypedValue tv) {
  auto const ad = static_cast<ArrayData*>(
    tl_heap->objMallocIndex(PackedArray::SmallSizeIndex)
  );
  ad->initHeader_16(
    HeaderKind::Packed,
    OneReference,
    PackedArray::packSizeIndexAndAuxBits(
      PackedArray::SmallSizeIndex,
      ArrayData::kNotDVArray
    )
  );
  ad->m_sizeAndPos = 1; // size=1, pos=0

  auto elem = PackedArray::LvalUncheckedInt(ad, 0);
  tvCopy(tv, elem);

  assertx(ad->kind() == ArrayData::kPackedKind);
  assertx(ad->dvArray() == ArrayData::kNotDVArray);
  assertx(ad->m_size == 1);
  assertx(ad->m_pos == 0);
  assertx(ad->hasExactlyOneRef());
  assertx(PackedArray::checkInvariants(ad));
  return arr_lval { ad, elem };
}

NEVER_INLINE
arr_lval EmptyArray::MakePacked(TypedValue tv) {
  return MakePackedInl(tv);
}

/*
 * Helper for creating a single-element mixed array with a string key.
 *
 * Note: the key is not already incref'd, but the value must be.
 */
NEVER_INLINE
arr_lval EmptyArray::MakeMixed(StringData* key, TypedValue val) {
  auto const ad = MixedArray::reqAlloc(MixedArray::SmallScale);
  MixedArray::InitSmall(ad, 1/*size*/, 0/*nextIntKey*/);
  auto const data = ad->data();
  auto const hash = reinterpret_cast<int32_t*>(data + MixedArray::SmallSize);
  auto const khash = key->hash();
  auto const mask = MixedArray::SmallMask;
  hash[khash & mask] = 0;
  data[0].setStrKey(key, khash);
  ad->recordStrKey(key);

  auto& elem  = data[0].data;
  elem.m_data = val.m_data;
  elem.m_type = val.m_type;

  assertx(ad->m_size == 1);
  assertx(ad->m_pos == 0);
  assertx(ad->m_scale == MixedArray::SmallScale);
  assertx(ad->kind() == ArrayData::kMixedKind);
  assertx(ad->hasExactlyOneRef());
  assertx(ad->m_used == 1);
  assertx(ad->checkInvariants());
  return arr_lval { ad, &elem };
}

/*
 * Creating a single-element mixed array with a integer key.  The
 * value is already incref'd.
 */
arr_lval EmptyArray::MakeMixed(int64_t key, TypedValue val) {
  auto const ad = MixedArray::reqAlloc(MixedArray::SmallScale);
  MixedArray::InitSmall(ad, 1/*size*/, (key >= 0) ? key + uint64_t{1} : 0);
  auto const data = ad->data();
  auto const hash = reinterpret_cast<int32_t*>(data + MixedArray::SmallSize);

  auto const mask = MixedArray::SmallMask;
  auto h = hash_int64(key);
  hash[h & mask] = 0;
  data[0].setIntKey(key, h);
  ad->recordIntKey();

  auto& elem  = data[0].data;
  elem.m_data = val.m_data;
  elem.m_type = val.m_type;

  assertx(ad->kind() == ArrayData::kMixedKind);
  assertx(ad->m_size == 1);
  assertx(ad->m_pos == 0);
  assertx(ad->hasExactlyOneRef());
  assertx(ad->m_scale == MixedArray::SmallScale);
  assertx(ad->m_used == 1);
  assertx(ad->checkInvariants());
  return arr_lval { ad, &elem };
}

//////////////////////////////////////////////////////////////////////

template<bool warn> ALWAYS_INLINE
arr_lval EmptyArray::LvalIntImpl(ArrayData*, int64_t k, bool) {
  if (warn && checkHACFalseyPromote()) {
    raise_hac_falsey_promote_notice("Lval on missing array element");
  }
  return k == 0 ? EmptyArray::MakePacked(make_tv<KindOfNull>())
                : EmptyArray::MakeMixed(k, make_tv<KindOfNull>());
}

template<bool warn> ALWAYS_INLINE
arr_lval EmptyArray::LvalStrImpl(ArrayData*, StringData* k, bool) {
  if (warn && checkHACFalseyPromote()) {
    raise_hac_falsey_promote_notice("Lval on missing array element");
  }
  return EmptyArray::MakeMixed(k, make_tv<KindOfNull>());
}

ArrayData* EmptyArray::SetInt(ArrayData*, int64_t k, Cell v) {
  // TODO(#3888164): we should make it so we don't need KindOfUninit checks
  if (v.m_type == KindOfUninit) v.m_type = KindOfNull;
  tvIncRefGen(v);
  auto const lval = k == 0 ? EmptyArray::MakePacked(v)
                           : EmptyArray::MakeMixed(k, v);
  return lval.arr;
}

ArrayData*
EmptyArray::SetStr(ArrayData*, StringData* k, Cell v) {
  tvIncRefGen(v);
  // TODO(#3888164): we should make it so we don't need KindOfUninit checks
  if (v.m_type == KindOfUninit) v.m_type = KindOfNull;
  return EmptyArray::MakeMixed(k, v).arr;
}

ArrayData* EmptyArray::SetWithRefInt(ArrayData* ad, int64_t k, TypedValue v) {
  if (checkHACRefBind() && tvIsReferenced(v)) {
    raiseHackArrCompatRefBind(k);
  }
  auto const lval = LvalIntImpl<false>(ad, k, ad->cowCheck());
  tvSetWithRef(v, lval);
  return lval.arr;
}

ArrayData* EmptyArray::SetWithRefStr(ArrayData* ad, StringData* k,
                                     TypedValue v) {
  if (checkHACRefBind() && tvIsReferenced(v)) {
    raiseHackArrCompatRefBind(k);
  }
  auto const lval = LvalStrImpl<false>(ad, k, ad->cowCheck());
  tvSetWithRef(v, lval);
  return lval.arr;
}

ArrayData* EmptyArray::RemoveInt(ArrayData* ad, int64_t) {
  return ad;
}

ArrayData* EmptyArray::RemoveStr(ArrayData* ad, const StringData*) {
  return ad;
}

arr_lval EmptyArray::LvalInt(ArrayData* ad, int64_t k, bool copy) {
  return LvalIntImpl<true>(ad, k, copy);
}

arr_lval EmptyArray::LvalStr(ArrayData* ad, StringData* k, bool copy) {
  return LvalStrImpl<true>(ad, k, copy);
}

arr_lval EmptyArray::LvalNew(ArrayData*, bool) {
  if (checkHACFalseyPromote()) {
    raise_hac_falsey_promote_notice("Lval on missing array element");
  }
  return EmptyArray::MakePacked(make_tv<KindOfNull>());
}

ArrayData* EmptyArray::Append(ArrayData*, Cell v) {
  tvIncRefGen(v);
  return EmptyArray::MakePackedInl(v).arr;
}

ArrayData* EmptyArray::AppendWithRef(ArrayData*, TypedValue v) {
  if (checkHACRefBind() && tvIsReferenced(v)) {
    raiseHackArrCompatRefNew();
  }
  auto tv = make_tv<KindOfNull>();
  tvAsVariant(&tv).setWithRef(v);
  return EmptyArray::MakePacked(tv).arr;
}

//////////////////////////////////////////////////////////////////////

ArrayData* EmptyArray::PlusEq(ArrayData*, const ArrayData* elems) {
  if (!elems->isPHPArray()) throwInvalidAdditionException(elems);
  elems->incRefCount();
  return const_cast<ArrayData*>(elems);
}

ArrayData* EmptyArray::Merge(ArrayData*, const ArrayData* elems) {
  if (elems->isNotDVArray()) {
    // Packed arrays don't need renumbering, so don't make a copy.
    if (elems->isPacked()) {
      elems->incRefCount();
      return const_cast<ArrayData*>(elems);
    }
    // Fast path the common case that elems is mixed.
    if (elems->isMixed()) {
      auto const copy = MixedArray::Copy(elems);
      assertx(copy != elems);
      MixedArray::Renumber(copy);
      return copy;
    }
  }
  auto copy = const_cast<ArrayData*>(elems)->toPHPArray(true);
  copy = copy == elems ? elems->copy() : copy;
  assertx(copy != elems);
  copy->renumber();
  return copy;
}

ArrayData* EmptyArray::PopOrDequeue(ArrayData* ad, Variant& value) {
  value = uninit_null();
  return ad;
}

ArrayData* EmptyArray::Prepend(ArrayData*, Cell v) {
  tvIncRefGen(v);
  return EmptyArray::MakePacked(v).arr;
}

ArrayData* EmptyArray::ToDict(ArrayData*, bool) {
  return ArrayData::CreateDict();
}

ArrayData* EmptyArray::ToVec(ArrayData*, bool) {
  return ArrayData::CreateVec();
}

ArrayData* EmptyArray::ToKeyset(ArrayData*, bool) {
  return ArrayData::CreateKeyset();
}

//////////////////////////////////////////////////////////////////////

}
