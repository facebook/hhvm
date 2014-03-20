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
#include "hphp/runtime/base/empty-array.h"

#include <utility>
#include <type_traits>

#include "hphp/util/assertions.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/tv-helpers.h"
#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/hphp-array.h"
#include "hphp/runtime/base/hphp-array-defs.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

std::aligned_storage<
  sizeof(ArrayData),
  alignof(ArrayData)
>::type s_theEmptyArray;

struct EmptyArray::Initializer {
  Initializer() {
    void* vpEmpty = &s_theEmptyArray;

    auto const ad   = static_cast<ArrayData*>(vpEmpty);
    ad->m_kind      = ArrayData::kEmptyKind;
    ad->m_size      = 0;
    ad->m_pos       = ArrayData::invalid_index;
    ad->m_count     = 0;
    ad->setStatic();
  }
};
EmptyArray::Initializer EmptyArray::s_initializer;

//////////////////////////////////////////////////////////////////////

void EmptyArray::Release(ArrayData*) {
  always_assert(!"never try to free the empty array");
}

/*
 * Used for NvGetInt, NvGetStr.  (We never contain the string or int.)
 *
 * Used for GetAPCHandle (we don't have one).
 */
void* EmptyArray::ReturnNull(...) {
  return nullptr;
}

/*
 * Used for ExistsInt, ExistsStr.  (We never contain the int or string.)
 */
bool EmptyArray::ReturnFalse(...) {
  return false;
}

/*
 * Used for IsVectorData (we're always trivially a vector).
 *
 * Used for Uksort, Usort, Uasort.  These functions return false only
 * if the user compare function modified they array, which it can't
 * here because we don't call it.
 */
bool EmptyArray::ReturnTrue(...) {
  return true;
}

void EmptyArray::NvGetKey(const ArrayData*, TypedValue* out, ssize_t pos) {
  // We have no valid positions---no one should call this function.
  not_reached();
}

const Variant& EmptyArray::GetValueRef(const ArrayData* ad, ssize_t pos) {
  // We have no valid positions---no one should call this function.
  not_reached();
}

/*
 * Used for RemoveInt, RemoveStr.  We don't every have the int or str,
 * so even if copy is true we can just return the same array.
 *
 * Used for EscalateForSort---we are already sorted by any imaginable
 * method of sorting, so the sort functions are no-ops, so we don't
 * need to copy.
 *
 * (TODO: verify nothing assumes that we /must/ copy when copy is
 * true.)
 */
ArrayData* EmptyArray::ReturnFirstArg(ArrayData* a, ...) {
  return a;
}

/*
 * Used for IterBegin and IterEnd.  We always return the invalid_index.
 */
ssize_t EmptyArray::ReturnInvalidIndex(const ArrayData*) {
  return ArrayData::invalid_index;
}

// Iterators can't be advanced or rewinded, because we have no valid
// iterators.
ssize_t EmptyArray::IterAdvance(const ArrayData*, ssize_t prev) {
  not_reached();
}
ssize_t EmptyArray::IterRewind(const ArrayData*, ssize_t prev) {
  not_reached();
}

// Strong iterating the empty array doesn't give back any elements.
bool EmptyArray::ValidMArrayIter(const ArrayData*, const MArrayIter& fp) {
  return false;
}
bool EmptyArray::AdvanceMArrayIter(ArrayData*, MArrayIter& fp) {
  not_reached();
}

/*
 * Don't do anything.
 *
 * Used for Ksort, Sort, and Asort.  The empty array is already
 * sorted, and these functions have no other side-effects.
 *
 * Used for Renumber---we're trivially numbered properly.
 */
void EmptyArray::NoOp(...) {}

// We're always already a static array.
void EmptyArray::OnSetEvalScalar(ArrayData*) { not_reached(); }
ArrayData* EmptyArray::NonSmartCopy(const ArrayData* ad) { not_reached(); }

//////////////////////////////////////////////////////////////////////

NEVER_INLINE
ArrayData* EmptyArray::Copy(const ArrayData*) {
  auto const mask = HphpArray::SmallMask;            // 3
  auto const cap  = HphpArray::computeMaxElms(mask); // 3
  auto const ad   = smartAllocArray(cap, mask);

  ad->m_kindAndSize = ArrayData::kPackedKind;
  ad->m_posAndCount = static_cast<uint32_t>(ArrayData::invalid_index);
  ad->m_capAndUsed  = cap;
  ad->m_tableMask   = mask;

  assert(ad->m_kind == ArrayData::kPackedKind);
  assert(ad->m_size == 0);
  assert(ad->m_pos == ArrayData::invalid_index);
  assert(ad->m_count == 0);
  assert(ad->m_cap == cap);
  assert(ad->m_used == 0);
  assert(ad->checkInvariants());
  return ad;
}

ArrayData* EmptyArray::CopyWithStrongIterators(const ArrayData* ad) {
  // We can never have strong iterators, so we don't need to do
  // anything extra.
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
 * with one element.  The element is transfered into the array (should
 * already be incref'd).
 */
ALWAYS_INLINE
std::pair<ArrayData*,TypedValue*> EmptyArray::MakePackedInl(TypedValue tv) {
  auto const mask = HphpArray::SmallMask;            // 3
  auto const cap  = HphpArray::computeMaxElms(mask); // 3
  auto const ad   = smartAllocArray(cap, mask);

  ad->m_kindAndSize = uint64_t{1} << 32 | ArrayData::kPackedKind;
  ad->m_posAndCount = 0;
  ad->m_capAndUsed  = uint64_t{1} << 32 | cap;
  ad->m_tableMask   = mask;

  auto& lval  = reinterpret_cast<HphpArray::Elm*>(ad + 1)[0].data;
  lval.m_data = tv.m_data;
  lval.m_type = tv.m_type;

  assert(ad->m_kind == ArrayData::kPackedKind);
  assert(ad->m_size == 1);
  assert(ad->m_pos == 0);
  assert(ad->m_count == 0);
  assert(ad->m_cap == cap);
  assert(ad->m_used == 1);
  assert(ad->checkInvariants());
  return { ad, &lval };
}

NEVER_INLINE
std::pair<ArrayData*,TypedValue*> EmptyArray::MakePacked(TypedValue tv) {
  return MakePackedInl(tv);
}

/*
 * Helper for creating a single-element mixed array with a string key.
 *
 * Note: the key is not already incref'd, but the value must be.
 */
NEVER_INLINE
std::pair<ArrayData*,TypedValue*>
EmptyArray::MakeMixed(StringData* key, TypedValue val) {
  auto const mask = HphpArray::SmallMask;            // 3
  auto const cap  = HphpArray::computeMaxElms(mask); // 3
  auto const ad   = smartAllocArray(cap, mask);

  ad->m_kindAndSize = uint64_t{1} << 32 | ArrayData::kMixedKind;
  ad->m_posAndCount = 0;
  ad->m_capAndUsed  = uint64_t{1} << 32 | cap;
  ad->m_tableMask   = mask;
  ad->m_nextKI      = 0;
  ad->m_hLoad       = 1;

  auto const data = reinterpret_cast<HphpArray::Elm*>(ad + 1);
  auto const hash = reinterpret_cast<int32_t*>(data + cap);

  assert(mask + 1 == 4);
  auto const emptyVal = int64_t{HphpArray::Empty};
  reinterpret_cast<int64_t*>(hash)[0] = emptyVal;
  reinterpret_cast<int64_t*>(hash)[1] = emptyVal;

  auto const khash = key->hash();
  hash[khash & mask] = 0;
  data[0].setStrKey(key, khash);

  auto& lval  = data[0].data;
  lval.m_data = val.m_data;
  lval.m_type = val.m_type;

  assert(ad->m_kind == ArrayData::kMixedKind);
  assert(ad->m_size == 1);
  assert(ad->m_pos == 0);
  assert(ad->m_count == 0);
  assert(ad->m_cap == cap);
  assert(ad->m_used == 1);
  assert(ad->checkInvariants());
  return { ad, &lval };
}

/*
 * Creating a single-element mixed array with a integer key.  The
 * value is already incref'd.
 */
std::pair<ArrayData*,TypedValue*>
EmptyArray::MakeMixed(int64_t key, TypedValue val) {
  auto const mask = HphpArray::SmallMask;            // 3
  auto const cap  = HphpArray::computeMaxElms(mask); // 3
  auto const ad   = smartAllocArray(cap, mask);

  ad->m_kindAndSize = uint64_t{1} << 32 | ArrayData::kMixedKind;
  ad->m_posAndCount = 0;
  ad->m_capAndUsed  = uint64_t{1} << 32 | cap;
  ad->m_tableMask   = mask;
  ad->m_nextKI      = key + 1;
  ad->m_hLoad       = 1;

  auto const data = reinterpret_cast<HphpArray::Elm*>(ad + 1);
  auto const hash = reinterpret_cast<int32_t*>(data + cap);

  assert(mask + 1 == 4);
  auto const emptyVal = int64_t{HphpArray::Empty};
  reinterpret_cast<int64_t*>(hash)[0] = emptyVal;
  reinterpret_cast<int64_t*>(hash)[1] = emptyVal;

  hash[key & mask] = 0;
  data[0].setIntKey(key);

  auto& lval  = data[0].data;
  lval.m_data = val.m_data;
  lval.m_type = val.m_type;

  assert(ad->m_kind == ArrayData::kMixedKind);
  assert(ad->m_size == 1);
  assert(ad->m_pos == 0);
  assert(ad->m_count == 0);
  assert(ad->m_cap == cap);
  assert(ad->m_used == 1);
  assert(ad->checkInvariants());
  return { ad, &lval };
}

//////////////////////////////////////////////////////////////////////

ArrayData* EmptyArray::SetInt(ArrayData*, int64_t k, const Variant& v, bool) {
  auto c = *v.asCell();
  tvRefcountedIncRef(&c);
  auto const ret = k == 0 ? EmptyArray::MakePacked(c)
                          : EmptyArray::MakeMixed(k, c);
  return ret.first;
}

ArrayData* EmptyArray::SetStr(ArrayData*,
                              StringData* k,
                              const Variant& v,
                              bool copy) {
  auto val = *v.asCell();
  tvRefcountedIncRef(&val);
  return EmptyArray::MakeMixed(k, val).first;
}

ArrayData* EmptyArray::LvalInt(ArrayData*, int64_t k, Variant*& retVar, bool) {
  auto const ret = k == 0 ? EmptyArray::MakePacked(make_tv<KindOfNull>())
                          : EmptyArray::MakeMixed(k, make_tv<KindOfNull>());
  retVar = &tvAsVariant(ret.second);
  return ret.first;
}

ArrayData* EmptyArray::LvalStr(ArrayData*,
                               StringData* k,
                               Variant*& retVar,
                               bool) {
  auto const ret = EmptyArray::MakeMixed(k, make_tv<KindOfNull>());
  retVar = &tvAsVariant(ret.second);
  return ret.first;
}

ArrayData* EmptyArray::LvalNew(ArrayData*, Variant*& retVar, bool) {
  auto const ret = EmptyArray::MakePacked(make_tv<KindOfNull>());
  retVar = &tvAsVariant(ret.second);
  return ret.first;
}

ArrayData* EmptyArray::SetRefInt(ArrayData*,
                                 int64_t k,
                                 const Variant& var,
                                 bool) {
  auto ref = *var.asRef();
  tvIncRef(&ref);
  auto const ret = k == 0 ? EmptyArray::MakePacked(ref)
                          : EmptyArray::MakeMixed(k, ref);
  return ret.first;
}

ArrayData* EmptyArray::SetRefStr(ArrayData*,
                                 StringData* k,
                                 const Variant& var,
                                 bool) {
  auto ref = *var.asRef();
  tvIncRef(&ref);
  return EmptyArray::MakeMixed(k, ref).first;
}

ArrayData* EmptyArray::Append(ArrayData*, const Variant& vin, bool copy) {
  auto cell = *vin.asCell();
  tvRefcountedIncRef(&cell);
  return EmptyArray::MakePackedInl(cell).first;
}

ArrayData* EmptyArray::AppendRef(ArrayData*, const Variant& v, bool copy) {
  auto ref = *v.asRef();
  tvIncRef(&ref);
  return EmptyArray::MakePacked(ref).first;
}

ArrayData* EmptyArray::AppendWithRef(ArrayData*, const Variant& v, bool copy) {
  auto tv = make_tv<KindOfNull>();
  tvAsVariant(&tv).setWithRef(v);
  return EmptyArray::MakePacked(tv).first;
}

//////////////////////////////////////////////////////////////////////

ArrayData* EmptyArray::PlusEq(ArrayData*, const ArrayData* elems) {
  elems->incRefCount();
  return const_cast<ArrayData*>(elems);
}

ArrayData* EmptyArray::Merge(ArrayData*, const ArrayData* elems) {
  auto const ret = HphpArray::MakeReserve(HphpArray::SmallSize);
  auto const tmp = HphpArray::Merge(ret, elems);
  ret->release();
  return tmp;
}

ArrayData* EmptyArray::PopOrDequeue(ArrayData* ad, Variant& value) {
  value = uninit_null();
  return ad;
}

ArrayData* EmptyArray::Prepend(ArrayData*, const Variant& vin, bool) {
  auto cell = *vin.asCell();
  tvRefcountedIncRef(&cell);
  return EmptyArray::MakePacked(cell).first;
}

//////////////////////////////////////////////////////////////////////

ArrayData* EmptyArray::ZSetInt(ArrayData* ad, int64_t k, RefData* v) {
  auto const arr = HphpArray::MakeReserve(HphpArray::SmallSize);
  arr->m_count = 0;
  DEBUG_ONLY auto const tmp = arr->zSet(k, v);
  assert(tmp == arr);
  return arr;
}

ArrayData* EmptyArray::ZSetStr(ArrayData* ad, StringData* k, RefData* v) {
  auto const arr = HphpArray::MakeReserve(HphpArray::SmallSize);
  arr->m_count = 0;
  DEBUG_ONLY auto const tmp = arr->zSet(k, v);
  assert(tmp == arr);
  return arr;
}

ArrayData* EmptyArray::ZAppend(ArrayData* ad, RefData* v) {
  auto const arr = HphpArray::MakeReserve(HphpArray::SmallSize);
  arr->m_count = 0;
  DEBUG_ONLY auto const tmp = arr->zAppend(v);
  assert(tmp == arr);
  return arr;
}

//////////////////////////////////////////////////////////////////////

}
