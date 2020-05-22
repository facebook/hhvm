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

TypedValue EmptyArray::GetPosKey(const ArrayData*, ssize_t /*pos*/) {
  // We have no valid positions---no one should call this function.
  not_reached();
}

TypedValue EmptyArray::GetPosVal(const ArrayData* /*ad*/, ssize_t /*pos*/) {
  // We have no valid positions---no one should call this function.
  not_reached();
}

size_t EmptyArray::Vsize(const ArrayData*) { not_reached(); }

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
 * Helper for creating a single-element mixed array with a string key.
 *
 * Note: the key is not already incref'd, but the value must be.
 */
NEVER_INLINE
arr_lval EmptyArray::MakeMixedStr(StringData* key, TypedValue val) {
  auto const ad = MixedArray::reqAlloc(MixedArray::SmallScale);
  MixedArray::InitSmall(ad, 1/*size*/, 0/*nextIntKey*/);
  auto const data = ad->data();
  auto const hash = reinterpret_cast<int32_t*>(data + MixedArray::SmallSize);
  auto const khash = key->hash();
  auto const mask = MixedArray::SmallMask;
  hash[khash & mask] = 0;
  data[0].setStrKey(key, khash);
  ad->mutableKeyTypes()->recordStr(key);

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
arr_lval EmptyArray::MakeMixedInt(int64_t key, TypedValue val) {
  auto const ad = MixedArray::reqAlloc(MixedArray::SmallScale);
  MixedArray::InitSmall(ad, 1/*size*/, (key >= 0) ? key + uint64_t{1} : 0);
  auto const data = ad->data();
  auto const hash = reinterpret_cast<int32_t*>(data + MixedArray::SmallSize);

  auto const mask = MixedArray::SmallMask;
  auto h = hash_int64(key);
  hash[h & mask] = 0;
  data[0].setIntKey(key, h);
  ad->mutableKeyTypes()->recordInt();

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

ArrayData* EmptyArray::SetInt(ArrayData* ad, int64_t k, TypedValue v) {
  tvIncRefGen(v);
  return SetIntMove(ad, k, v);
}

ArrayData* EmptyArray::SetIntMove(ArrayData*, int64_t k, TypedValue v) {
  // TODO(#3888164): we should make it so we don't need KindOfUninit checks
  if (v.m_type == KindOfUninit) v.m_type = KindOfNull;
  return EmptyArray::MakeMixedInt(k, v).arr;
}

ArrayData* EmptyArray::SetStr(ArrayData* ad, StringData* k, TypedValue v) {
  tvIncRefGen(v);
  return SetStrMove(ad, k, v);
}

ArrayData* EmptyArray::SetStrMove(ArrayData*, StringData* k, TypedValue v) {
  // TODO(#3888164): we should make it so we don't need KindOfUninit checks
  if (v.m_type == KindOfUninit) v.m_type = KindOfNull;
  return EmptyArray::MakeMixedStr(k, v).arr;
}

ArrayData* EmptyArray::RemoveInt(ArrayData* ad, int64_t) {
  return ad;
}

ArrayData* EmptyArray::RemoveStr(ArrayData* ad, const StringData*) {
  return ad;
}

arr_lval EmptyArray::LvalInt(ArrayData*, int64_t) {
  throwMissingElementException("Lval");
}

arr_lval EmptyArray::LvalStr(ArrayData*, StringData*) {
  throwMissingElementException("Lval");
}

ArrayData* EmptyArray::Append(ArrayData*, TypedValue v) {
  tvIncRefGen(v);
  return EmptyArray::MakeMixedInt(0, v).arr;
}

//////////////////////////////////////////////////////////////////////

ArrayData* EmptyArray::PlusEq(ArrayData*, const ArrayData* elems) {
  if (!elems->isPHPArrayType()) throwInvalidAdditionException(elems);
  elems->incRefCount();
  return const_cast<ArrayData*>(elems);
}

ArrayData* EmptyArray::Merge(ArrayData*, const ArrayData* elems) {
  if (elems->isNotDVArray()) {
    // Packed arrays don't need renumbering, so don't make a copy.
    if (elems->isPackedKind()) {
      elems->incRefCount();
      return const_cast<ArrayData*>(elems);
    }
    // Fast path the common case that elems is mixed.
    if (elems->isMixedKind()) {
      auto const copy = MixedArray::Copy(elems);
      assertx(copy != elems);
      assertx(copy->hasExactlyOneRef());
      DEBUG_ONLY auto const escalated = MixedArray::Renumber(copy);
      assertx(escalated == copy);
      return copy;
    }
  }
  auto const copy = const_cast<ArrayData*>(elems)->toPHPArray(true);
  assertx(copy != elems);
  assertx(copy->hasExactlyOneRef());
  auto const escalated = copy->renumber();
  if (escalated != copy) copy->release();
  return escalated;
}

ArrayData* EmptyArray::PopOrDequeue(ArrayData* ad, Variant& value) {
  value = uninit_null();
  return ad;
}

ArrayData* EmptyArray::Prepend(ArrayData*, TypedValue v) {
  tvIncRefGen(v);
  return EmptyArray::MakeMixedInt(0, v).arr;
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
