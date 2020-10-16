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
#include "hphp/runtime/base/bespoke/logging-array.h"
#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/sort-flags.h"
#include "hphp/runtime/base/tv-refcount.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////////////

BespokeArray* BespokeArray::asBespoke(ArrayData* ad) {
  auto ret = reinterpret_cast<BespokeArray*>(ad);
  assertx(ret->checkInvariants());
  return ret;
}
const BespokeArray* BespokeArray::asBespoke(const ArrayData* ad) {
  return asBespoke(const_cast<ArrayData*>(ad));
}

BespokeLayout BespokeArray::layout() const {
  return BespokeLayout{bespoke::Layout::FromIndex(layoutIndex())};
}

bespoke::LayoutIndex BespokeArray::layoutIndex() const {
  return {safe_cast<uint16_t>(m_extra_hi16 & ~kExtraMagicBit.raw)};
}

const bespoke::LayoutFunctions* BespokeArray::vtable() const {
  return bespoke::Layout::FromIndex(layoutIndex())->vtable();
}

void BespokeArray::setLayoutIndex(bespoke::LayoutIndex index) {
  static_assert(bespoke::Layout::kMaxIndex.raw < kExtraMagicBit.raw);
  m_extra_hi16 = index.raw | kExtraMagicBit.raw;
}

size_t BespokeArray::heapSize() const {
  return vtable()->fnHeapSize(this);
}
void BespokeArray::scan(type_scan::Scanner& scan) const {
  return vtable()->fnScan(this, scan);
}

ArrayData* BespokeArray::ToVanilla(const ArrayData* ad, const char* reason) {
  return asBespoke(ad)->vtable()->fnEscalateToVanilla(ad, reason);
}

void BespokeArray::logReachEvent(TransID transId, uint32_t guardIdx) {
  if (layoutIndex() != bespoke::LoggingArray::GetLayoutIndex()) return;
  bespoke::LoggingArray::As(this)->logReachEvent(transId, guardIdx);
}

bool BespokeArray::checkInvariants() const {
  assertx(!isVanilla());
  assertx(kindIsValid());
  assertx(vtable() != nullptr);
  assertx(m_extra_hi16 & kExtraMagicBit.raw);
  return true;
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
  auto const vtable = asBespoke(ad)->vtable();
  auto const extra = uncountedAllocExtra(ad, hasApcTv);
  auto const bytes = vtable->fnHeapSize(ad);
  assertx(extra % 16 == 0);

  // "Help" out by copying the array's raw bytes to an uncounted allocation.
  auto const mem = static_cast<char*>(uncounted_malloc(bytes + extra));
  auto const result = reinterpret_cast<ArrayData*>(mem + extra);
  memcpy8(reinterpret_cast<char*>(result),
          reinterpret_cast<char*>(ad), bytes);
  auto const aux = ad->m_aux16 | (hasApcTv ? ArrayData::kHasApcTv : 0);
  result->initHeader_16(HeaderKind(ad->kind()), UncountedValue, aux);
  assertx(asBespoke(result)->layoutIndex() == asBespoke(ad)->layoutIndex());

  vtable->fnConvertToUncounted(result, seen);
  if (updateSeen) (*seen)[ad] = result;
  return result;
}

void BespokeArray::ReleaseUncounted(ArrayData* ad) {
  if (!ad->uncountedDecRef()) return;
  auto const vtable = asBespoke(ad)->vtable();
  vtable->fnReleaseUncounted(ad);
  if (APCStats::IsCreated()) {
    APCStats::getAPCStats().removeAPCUncountedBlock();
  }
  auto const bytes = vtable->fnHeapSize(ad);
  auto const extra = uncountedAllocExtra(ad, ad->hasApcTv());
  uncounted_sized_free(reinterpret_cast<char*>(ad) - extra, bytes + extra);
}

//////////////////////////////////////////////////////////////////////////////

// ArrayData interface
void BespokeArray::Release(ArrayData* ad) {
  asBespoke(ad)->vtable()->fnRelease(ad);
}
bool BespokeArray::IsVectorData(const ArrayData* ad) {
  return asBespoke(ad)->vtable()->fnIsVectorData(ad);
}

// RO access
TypedValue BespokeArray::NvGetInt(const ArrayData* ad, int64_t key) {
  return asBespoke(ad)->vtable()->fnGetInt(ad, key);
}
TypedValue BespokeArray::NvGetStr(const ArrayData* ad, const StringData* key) {
  return asBespoke(ad)->vtable()->fnGetStr(ad, key);
}
TypedValue BespokeArray::GetPosKey(const ArrayData* ad, ssize_t pos) {
  return asBespoke(ad)->vtable()->fnGetKey(ad, pos);
}
TypedValue BespokeArray::GetPosVal(const ArrayData* ad, ssize_t pos) {
  return asBespoke(ad)->vtable()->fnGetVal(ad, pos);
}
ssize_t BespokeArray::NvGetIntPos(const ArrayData* ad, int64_t key) {
  return asBespoke(ad)->vtable()->fnGetIntPos(ad, key);
}
ssize_t BespokeArray::NvGetStrPos(const ArrayData* ad, const StringData* key) {
  return asBespoke(ad)->vtable()->fnGetStrPos(ad, key);
}
bool BespokeArray::ExistsInt(const ArrayData* ad, int64_t key) {
  return NvGetInt(ad, key).is_init();
}
bool BespokeArray::ExistsStr(const ArrayData* ad, const StringData* key) {
  return NvGetStr(ad, key).is_init();
}

// iteration
ssize_t BespokeArray::IterBegin(const ArrayData* ad) {
  return asBespoke(ad)->vtable()->fnIterBegin(ad);
}
ssize_t BespokeArray::IterLast(const ArrayData* ad) {
  return asBespoke(ad)->vtable()->fnIterLast(ad);
}
ssize_t BespokeArray::IterEnd(const ArrayData* ad) {
  return asBespoke(ad)->vtable()->fnIterEnd(ad);
}
ssize_t BespokeArray::IterAdvance(const ArrayData* ad, ssize_t pos) {
  return asBespoke(ad)->vtable()->fnIterAdvance(ad, pos);
}
ssize_t BespokeArray::IterRewind(const ArrayData* ad, ssize_t pos) {
  return asBespoke(ad)->vtable()->fnIterRewind(ad, pos);
}

// RW access
arr_lval BespokeArray::LvalInt(ArrayData* ad, int64_t key) {
  return asBespoke(ad)->vtable()->fnLvalInt(ad, key);
}
arr_lval BespokeArray::LvalStr(ArrayData* ad, StringData* key) {
  return asBespoke(ad)->vtable()->fnLvalStr(ad, key);
}
arr_lval BespokeArray::ElemInt(ArrayData* ad, int64_t key) {
  return asBespoke(ad)->vtable()->fnElemInt(ad, key);
}
arr_lval BespokeArray::ElemStr(ArrayData* ad, StringData* key) {
  return asBespoke(ad)->vtable()->fnElemStr(ad, key);
}

// insertion
ArrayData* BespokeArray::SetInt(ArrayData* ad, int64_t key, TypedValue v) {
  return asBespoke(ad)->vtable()->fnSetInt(ad, key, v);
}
ArrayData* BespokeArray::SetStr(ArrayData* ad, StringData* key, TypedValue v) {
  return asBespoke(ad)->vtable()->fnSetStr(ad, key, v);
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
  return asBespoke(ad)->vtable()->fnRemoveInt(ad, key);
}
ArrayData* BespokeArray::RemoveStr(ArrayData* ad, const StringData* key) {
  return asBespoke(ad)->vtable()->fnRemoveStr(ad, key);
}

// sorting
ArrayData* BespokeArray::EscalateForSort(ArrayData* ad, SortFunction sf) {
  auto const vad = asBespoke(ad)->vtable()->fnEscalateToVanilla(
    ad, sortFunctionName(sf)
  );
  return vad->escalateForSort(sf);
}

// high-level ops
ArrayData* BespokeArray::Append(ArrayData* ad, TypedValue v) {
  return asBespoke(ad)->vtable()->fnAppend(ad, v);
}
ArrayData* BespokeArray::Pop(ArrayData* ad, Variant& out) {
  return asBespoke(ad)->vtable()->fnPop(ad, out);
}
void BespokeArray::OnSetEvalScalar(ArrayData*) {
  always_assert(false);
}

// copies and conversions
ArrayData* BespokeArray::CopyStatic(const ArrayData*) {
  always_assert(false);
}
ArrayData* BespokeArray::ToDVArray(ArrayData* ad, bool copy) {
  return asBespoke(ad)->vtable()->fnToDVArray(ad, copy);
}
ArrayData* BespokeArray::ToHackArr(ArrayData* ad, bool copy) {
  return asBespoke(ad)->vtable()->fnToHackArr(ad, copy);
}
ArrayData* BespokeArray::SetLegacyArray(ArrayData* ad, bool copy, bool legacy) {
  return asBespoke(ad)->vtable()->fnSetLegacyArray(ad, copy, legacy);
}

//////////////////////////////////////////////////////////////////////////////

}
