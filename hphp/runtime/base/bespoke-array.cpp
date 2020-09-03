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

#include "hphp/runtime/base/apc-stats.h"
#include "hphp/runtime/base/array-data-defs.h"
#include "hphp/runtime/base/bespoke-array.h"
#include "hphp/runtime/base/bespoke/layout.h"
#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/sort-flags.h"
#include "hphp/runtime/base/tv-refcount.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////////////

BespokeArray* BespokeArray::asBespoke(ArrayData* ad) {
  assertx(!ad->isVanilla());
  return reinterpret_cast<BespokeArray*>(ad);
}
const BespokeArray* BespokeArray::asBespoke(const ArrayData* ad) {
  return asBespoke(const_cast<ArrayData*>(ad));
}

BespokeLayout BespokeArray::layout() const {
  return BespokeLayout{layoutRaw()};
}

const bespoke::Layout* BespokeArray::layoutRaw() const {
  return bespoke::layoutForIndex(static_cast<uint16_t>(m_extra));
}

void BespokeArray::setLayoutRaw(const bespoke::Layout* layout) {
  m_extra = layout->index();
}

size_t BespokeArray::heapSize() const {
  return layoutRaw()->heapSize(this);
}
void BespokeArray::scan(type_scan::Scanner& scan) const {
  return layoutRaw()->scan(this, scan);
}

ArrayData* BespokeArray::ToVanilla(const ArrayData* ad, const char* reason) {
  return BespokeArray::asBespoke(ad)->layoutRaw()->escalateToVanilla(ad, reason);
}

//////////////////////////////////////////////////////////////////////////////

ArrayData* BespokeArray::MakeUncounted(ArrayData* ad, bool hasApcTv,
                                       DataWalker::PointerMap* seen) {
  assertx(ad->isRefCounted());
  auto const updateSeen = seen && ad->hasMultipleRefs();
  if (updateSeen) {
    auto const it = seen->find(ad);
    assertx(it != seen->end());
    if (auto const result = static_cast<ArrayData*>(it->second)) {
      if (result->uncountedIncRef()) return result;
    }
  }

  if (APCStats::IsCreated()) {
    APCStats::getAPCStats().addAPCUncountedBlock();
  }
  auto const layout = BespokeArray::asBespoke(ad)->layoutRaw();
  auto const extra = uncountedAllocExtra(ad, hasApcTv);
  auto const bytes = layout->heapSize(ad);
  assertx(bytes % 16 == 0);

  // "Help" out by copying the array's raw bytes to an uncounted allocation.
  auto const mem = static_cast<char*>(uncounted_malloc(bytes + extra));
  auto const result = reinterpret_cast<ArrayData*>(mem + extra);
  memcpy16_inline(reinterpret_cast<char*>(result),
                  reinterpret_cast<char*>(ad), bytes);
  auto const aux = ad->auxBits() | (hasApcTv ? ArrayData::kHasApcTv : 0);
  result->initHeader_16(HeaderKind(ad->kind()), UncountedValue, aux);
  assertx(BespokeArray::asBespoke(result)->layoutRaw() == layout);

  layout->convertToUncounted(result, seen);
  if (updateSeen) (*seen)[ad] = result;
  return result;
}

void BespokeArray::ReleaseUncounted(ArrayData* ad) {
  if (!ad->uncountedDecRef()) return;
  auto const layout = BespokeArray::asBespoke(ad)->layoutRaw();
  asBespoke(ad)->layoutRaw()->releaseUncounted(ad);
  if (APCStats::IsCreated()) {
    APCStats::getAPCStats().removeAPCUncountedBlock();
  }
  auto const extra = uncountedAllocExtra(ad, ad->hasApcTv());
  auto const bytes = layout->heapSize(ad);
  uncounted_sized_free(reinterpret_cast<char*>(ad) - extra, bytes + extra);
}

//////////////////////////////////////////////////////////////////////////////

// ArrayData interface
void BespokeArray::Release(ArrayData* ad) {
  asBespoke(ad)->layoutRaw()->release(ad);
}
bool BespokeArray::IsVectorData(const ArrayData* ad) {
  return asBespoke(ad)->layoutRaw()->isVectorData(ad);
}

// RO access
TypedValue BespokeArray::NvGetInt(const ArrayData* ad, int64_t key) {
  return asBespoke(ad)->layoutRaw()->getInt(ad, key);
}
TypedValue BespokeArray::NvGetStr(const ArrayData* ad, const StringData* key) {
  return asBespoke(ad)->layoutRaw()->getStr(ad, key);
}
TypedValue BespokeArray::GetPosKey(const ArrayData* ad, ssize_t pos) {
  return asBespoke(ad)->layoutRaw()->getKey(ad, pos);
}
TypedValue BespokeArray::GetPosVal(const ArrayData* ad, ssize_t pos) {
  return asBespoke(ad)->layoutRaw()->getVal(ad, pos);
}
ssize_t BespokeArray::NvGetIntPos(const ArrayData* ad, int64_t key) {
  return asBespoke(ad)->layoutRaw()->getIntPos(ad, key);
}
ssize_t BespokeArray::NvGetStrPos(const ArrayData* ad, const StringData* key) {
  return asBespoke(ad)->layoutRaw()->getStrPos(ad, key);
}
bool BespokeArray::ExistsInt(const ArrayData* ad, int64_t key) {
  return NvGetInt(ad, key).is_init();
}
bool BespokeArray::ExistsStr(const ArrayData* ad, const StringData* key) {
  return NvGetStr(ad, key).is_init();
}

// RW access
arr_lval BespokeArray::LvalInt(ArrayData* ad, int64_t key) {
  return asBespoke(ad)->layoutRaw()->lvalInt(ad, key);
}
arr_lval BespokeArray::LvalStr(ArrayData* ad, StringData* key) {
  return asBespoke(ad)->layoutRaw()->lvalStr(ad, key);
}

// insertion
ArrayData* BespokeArray::SetInt(ArrayData* ad, int64_t key, TypedValue v) {
  return asBespoke(ad)->layoutRaw()->setInt(ad, key, v);
}
ArrayData* BespokeArray::SetStr(ArrayData* ad, StringData* key, TypedValue v) {
  return asBespoke(ad)->layoutRaw()->setStr(ad, key, v);
}
ArrayData* BespokeArray::SetIntMove(ArrayData* ad, int64_t key, TypedValue val) {
  auto const result = SetInt(ad, key, val);
  if (result != ad) decRefArr(ad);
  tvDecRefGen(val);
  return result;
}
ArrayData* BespokeArray::SetStrMove(ArrayData* ad, StringData* key, TypedValue val) {
  auto const result = SetStr(ad, key, val);
  if (result != ad) decRefArr(ad);
  tvDecRefGen(val);
  return result;
}

// deletion
ArrayData* BespokeArray::RemoveInt(ArrayData* ad, int64_t key) {
  return asBespoke(ad)->layoutRaw()->removeInt(ad, key);
}
ArrayData* BespokeArray::RemoveStr(ArrayData* ad, const StringData* key) {
  return asBespoke(ad)->layoutRaw()->removeStr(ad, key);
}

// iteration
ssize_t BespokeArray::IterBegin(const ArrayData* ad) {
  return asBespoke(ad)->layoutRaw()->iterBegin(ad);
}
ssize_t BespokeArray::IterLast(const ArrayData* ad) {
  return asBespoke(ad)->layoutRaw()->iterLast(ad);
}
ssize_t BespokeArray::IterEnd(const ArrayData* ad) {
  return asBespoke(ad)->layoutRaw()->iterEnd(ad);
}
ssize_t BespokeArray::IterAdvance(const ArrayData* ad, ssize_t pos) {
  return asBespoke(ad)->layoutRaw()->iterAdvance(ad, pos);
}
ssize_t BespokeArray::IterRewind(const ArrayData* ad, ssize_t pos) {
  return asBespoke(ad)->layoutRaw()->iterRewind(ad, pos);
}

// sorting
ArrayData* BespokeArray::EscalateForSort(ArrayData* ad, SortFunction sf) {
  auto const vad = asBespoke(ad)->layoutRaw()->escalateToVanilla(
    ad, sortFunctionName(sf)
  );
  return vad->escalateForSort(sf);
}

// high-level ops
ArrayData* BespokeArray::Append(ArrayData* ad, TypedValue v) {
  return asBespoke(ad)->layoutRaw()->append(ad, v);
}
ArrayData* BespokeArray::Prepend(ArrayData* ad, TypedValue v) {
  return asBespoke(ad)->layoutRaw()->prepend(ad, v);
}
ArrayData* BespokeArray::Merge(ArrayData* ad, const ArrayData* elems) {
  return asBespoke(ad)->layoutRaw()->merge(ad, elems);
}
ArrayData* BespokeArray::Pop(ArrayData* ad, Variant& out) {
  return asBespoke(ad)->layoutRaw()->pop(ad, out);
}
ArrayData* BespokeArray::Dequeue(ArrayData* ad, Variant& out) {
  return asBespoke(ad)->layoutRaw()->dequeue(ad, out);
}
ArrayData* BespokeArray::Renumber(ArrayData* ad) {
  return asBespoke(ad)->layoutRaw()->renumber(ad);
}
void BespokeArray::OnSetEvalScalar(ArrayData*) {
  always_assert(false);
}

// copies and conversions
ArrayData* BespokeArray::Copy(const ArrayData* ad) {
  return asBespoke(ad)->layoutRaw()->copy(ad);
}
ArrayData* BespokeArray::CopyStatic(const ArrayData*) {
  always_assert(false);
}
ArrayData* BespokeArray::ToVArray(ArrayData* ad, bool copy) {
  return asBespoke(ad)->layoutRaw()->toVArray(ad, copy);
}
ArrayData* BespokeArray::ToDArray(ArrayData* ad, bool copy) {
  return asBespoke(ad)->layoutRaw()->toDArray(ad, copy);
}
ArrayData* BespokeArray::ToVec(ArrayData* ad, bool copy) {
  return asBespoke(ad)->layoutRaw()->toVec(ad, copy);
}
ArrayData* BespokeArray::ToDict(ArrayData* ad, bool copy) {
  return asBespoke(ad)->layoutRaw()->toDict(ad, copy);
}
ArrayData* BespokeArray::ToKeyset(ArrayData* ad, bool copy) {
  return asBespoke(ad)->layoutRaw()->toKeyset(ad, copy);
}

// flags
void BespokeArray::SetLegacyArrayInPlace(ArrayData* ad, bool legacy) {
  assertx(ad->hasExactlyOneRef());
  auto const bad = asBespoke(ad);
  bad->layoutRaw()->setLegacyArrayInPlace(ad, legacy);
  bad->m_aux16 = (bad->m_aux16 & ~kLegacyArray)
                 | (legacy ? kLegacyArray : 0);
}

//////////////////////////////////////////////////////////////////////////////

}
