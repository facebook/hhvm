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

const bespoke::Layout* BespokeArray::layout() const {
  return bespoke::layoutForIndex(~m_size);
}
void BespokeArray::setLayout(const bespoke::Layout* layout) {
  m_sizeAndPos = uint32_t(~layout->index());
}

size_t BespokeArray::heapSize() const {
  return layout()->heapSize(this);
}
void BespokeArray::scan(type_scan::Scanner& scan) const {
  return layout()->scan(this, scan);
}

ArrayData* BespokeArray::ToVanilla(const ArrayData* ad, const char* reason) {
  return BespokeArray::asBespoke(ad)->layout()->escalateToVanilla(ad, reason);
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
  auto const layout = BespokeArray::asBespoke(ad)->layout();
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
  assertx(BespokeArray::asBespoke(result)->layout() == layout);

  layout->convertToUncounted(result, seen);
  if (updateSeen) (*seen)[ad] = result;
  return result;
}

void BespokeArray::ReleaseUncounted(ArrayData* ad) {
  if (!ad->uncountedDecRef()) return;
  auto const layout = BespokeArray::asBespoke(ad)->layout();
  asBespoke(ad)->layout()->releaseUncounted(ad);
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
  asBespoke(ad)->layout()->release(ad);
}
size_t BespokeArray::Vsize(const ArrayData* ad) {
  return asBespoke(ad)->layout()->size(ad);
}
bool BespokeArray::IsVectorData(const ArrayData* ad) {
  return asBespoke(ad)->layout()->isVectorData(ad);
}

// RO access
TypedValue BespokeArray::NvGetInt(const ArrayData* ad, int64_t key) {
  return asBespoke(ad)->layout()->getInt(ad, key);
}
TypedValue BespokeArray::NvGetStr(const ArrayData* ad, const StringData* key) {
  return asBespoke(ad)->layout()->getStr(ad, key);
}
TypedValue BespokeArray::GetPosKey(const ArrayData* ad, ssize_t pos) {
  return asBespoke(ad)->layout()->getKey(ad, pos);
}
TypedValue BespokeArray::GetPosVal(const ArrayData* ad, ssize_t pos) {
  return asBespoke(ad)->layout()->getVal(ad, pos);
}
ssize_t BespokeArray::NvGetIntPos(const ArrayData* ad, int64_t key) {
  return asBespoke(ad)->layout()->getIntPos(ad, key);
}
ssize_t BespokeArray::NvGetStrPos(const ArrayData* ad, const StringData* key) {
  return asBespoke(ad)->layout()->getStrPos(ad, key);
}
bool BespokeArray::ExistsInt(const ArrayData* ad, int64_t key) {
  return NvGetInt(ad, key).is_init();
}
bool BespokeArray::ExistsStr(const ArrayData* ad, const StringData* key) {
  return NvGetStr(ad, key).is_init();
}

// RW access
arr_lval BespokeArray::LvalInt(ArrayData* ad, int64_t key) {
  return asBespoke(ad)->layout()->lvalInt(ad, key);
}
arr_lval BespokeArray::LvalStr(ArrayData* ad, StringData* key) {
  return asBespoke(ad)->layout()->lvalStr(ad, key);
}

// insertion
ArrayData* BespokeArray::SetInt(ArrayData* ad, int64_t key, TypedValue v) {
  return asBespoke(ad)->layout()->setInt(ad, key, v);
}
ArrayData* BespokeArray::SetStr(ArrayData* ad, StringData* key, TypedValue v) {
  return asBespoke(ad)->layout()->setStr(ad, key, v);
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
  return asBespoke(ad)->layout()->removeInt(ad, key);
}
ArrayData* BespokeArray::RemoveStr(ArrayData* ad, const StringData* key) {
  return asBespoke(ad)->layout()->removeStr(ad, key);
}

// iteration
ssize_t BespokeArray::IterBegin(const ArrayData* ad) {
  return asBespoke(ad)->layout()->iterBegin(ad);
}
ssize_t BespokeArray::IterLast(const ArrayData* ad) {
  return asBespoke(ad)->layout()->iterLast(ad);
}
ssize_t BespokeArray::IterEnd(const ArrayData* ad) {
  return asBespoke(ad)->layout()->iterEnd(ad);
}
ssize_t BespokeArray::IterAdvance(const ArrayData* ad, ssize_t pos) {
  return asBespoke(ad)->layout()->iterAdvance(ad, pos);
}
ssize_t BespokeArray::IterRewind(const ArrayData* ad, ssize_t pos) {
  return asBespoke(ad)->layout()->iterRewind(ad, pos);
}

// sorting
ArrayData* BespokeArray::EscalateForSort(ArrayData* ad, SortFunction sf) {
  auto const vad = asBespoke(ad)->layout()->escalateToVanilla(
    ad, sortFunctionName(sf)
  );
  return vad->escalateForSort(sf);
}

// high-level ops
ArrayData* BespokeArray::Append(ArrayData* ad, TypedValue v) {
  return asBespoke(ad)->layout()->append(ad, v);
}
ArrayData* BespokeArray::Prepend(ArrayData* ad, TypedValue v) {
  return asBespoke(ad)->layout()->prepend(ad, v);
}
ArrayData* BespokeArray::Merge(ArrayData* ad, const ArrayData* elems) {
  return asBespoke(ad)->layout()->merge(ad, elems);
}
ArrayData* BespokeArray::Pop(ArrayData* ad, Variant& out) {
  return asBespoke(ad)->layout()->pop(ad, out);
}
ArrayData* BespokeArray::Dequeue(ArrayData* ad, Variant& out) {
  return asBespoke(ad)->layout()->dequeue(ad, out);
}
ArrayData* BespokeArray::Renumber(ArrayData* ad) {
  return asBespoke(ad)->layout()->renumber(ad);
}
void BespokeArray::OnSetEvalScalar(ArrayData*) {
  always_assert(false);
}

// copies and conversions
ArrayData* BespokeArray::Copy(const ArrayData* ad) {
  return asBespoke(ad)->layout()->copy(ad);
}
ArrayData* BespokeArray::CopyStatic(const ArrayData*) {
  always_assert(false);
}
ArrayData* BespokeArray::ToVArray(ArrayData* ad, bool copy) {
  return asBespoke(ad)->layout()->toVArray(ad, copy);
}
ArrayData* BespokeArray::ToDArray(ArrayData* ad, bool copy) {
  return asBespoke(ad)->layout()->toDArray(ad, copy);
}
ArrayData* BespokeArray::ToVec(ArrayData* ad, bool copy) {
  return asBespoke(ad)->layout()->toVec(ad, copy);
}
ArrayData* BespokeArray::ToDict(ArrayData* ad, bool copy) {
  return asBespoke(ad)->layout()->toDict(ad, copy);
}
ArrayData* BespokeArray::ToKeyset(ArrayData* ad, bool copy) {
  return asBespoke(ad)->layout()->toKeyset(ad, copy);
}

// flags
void BespokeArray::SetLegacyArrayInPlace(ArrayData* ad, bool legacy) {
  assertx(ad->hasExactlyOneRef());
  auto const bad = asBespoke(ad);
  bad->layout()->setLegacyArrayInPlace(ad, legacy);
  bad->m_aux16 = (bad->m_aux16 & ~kLegacyArray)
                 | (legacy ? kLegacyArray : 0);
}

//////////////////////////////////////////////////////////////////////////////

}
