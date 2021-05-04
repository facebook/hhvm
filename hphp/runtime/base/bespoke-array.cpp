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

#include "hphp/runtime/base/array-data-defs.h"
#include "hphp/runtime/base/bespoke-array.h"
#include "hphp/runtime/base/bespoke/layout.h"
#include "hphp/runtime/base/bespoke/logging-array.h"
#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/sort-flags.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/tv-uncounted.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////////////

namespace {
using bespoke::g_layout_funcs;

size_t getVtableIndex(const ArrayData* ad) {
  auto const index = BespokeArray::asBespoke(ad)->layoutIndex();
  auto const result = index.byte() & bespoke::kBespokeVtableMask;
  if constexpr (debug) {
    DEBUG_ONLY auto const vtable = bespoke::Layout::FromIndex(index)->vtable();
    DEBUG_ONLY auto const release = g_layout_funcs.fnRelease[result];
    assertx(vtable != nullptr);
    assertx(release != nullptr);
    assertx(release == vtable->fnRelease);
  }
  return result;
}
}

BespokeArray* BespokeArray::asBespoke(ArrayData* ad) {
  auto ret = reinterpret_cast<BespokeArray*>(ad);
  assertx(ret->checkInvariants());
  return ret;
}
const BespokeArray* BespokeArray::asBespoke(const ArrayData* ad) {
  return asBespoke(const_cast<ArrayData*>(ad));
}

bespoke::LayoutIndex BespokeArray::layoutIndex() const {
  return m_layout_index;
}

const bespoke::LayoutFunctions* BespokeArray::vtable() const {
  return bespoke::ConcreteLayout::FromConcreteIndex(layoutIndex())->vtable();
}

void BespokeArray::setLayoutIndex(bespoke::LayoutIndex index) {
  m_layout_index = index;
}

NO_PROFILING
size_t BespokeArray::heapSize() const {
  return g_layout_funcs.fnHeapSize[getVtableIndex(this)](this);
}

NO_PROFILING
void BespokeArray::scan(type_scan::Scanner& scan) const {
  return g_layout_funcs.fnScan[getVtableIndex(this)](this, scan);
}

NO_PROFILING
ArrayData* BespokeArray::ToVanilla(const ArrayData* ad, const char* reason) {
  return g_layout_funcs.fnEscalateToVanilla[getVtableIndex(ad)](ad, reason);
}

bool BespokeArray::checkInvariants() const {
  assertx(!isVanilla());
  assertx(kindIsValid());
  assertx(!isSampledArray());
  static_assert(ArrayData::kVanillaLayoutIndex.raw == uint16_t(-1));
  assertx(m_layout_index != ArrayData::kVanillaLayoutIndex);
  return true;
}

//////////////////////////////////////////////////////////////////////////////

ArrayData* BespokeArray::MakeUncounted(
    ArrayData* ad, const MakeUncountedEnv& env, bool hasApcTv) {
  assertx(ad->isRefCounted());
  auto const vindex = getVtableIndex(ad);
  auto const extra = uncountedAllocExtra(ad, hasApcTv);
  auto const bytes = g_layout_funcs.fnHeapSize[vindex](ad);
  assertx(extra % 16 == 0);

  // "Help" out by copying the array's raw bytes to an uncounted allocation.
  auto const mem = static_cast<char*>(AllocUncounted(bytes + extra));
  auto const result = reinterpret_cast<ArrayData*>(mem + extra);
  memcpy8(reinterpret_cast<char*>(result),
          reinterpret_cast<char*>(ad), bytes);
  auto const aux = ad->m_aux16 | (hasApcTv ? ArrayData::kHasApcTv : 0);
  result->initHeader_16(HeaderKind(ad->kind()), UncountedValue, aux);
  assertx(asBespoke(result)->layoutIndex() == asBespoke(ad)->layoutIndex());

  g_layout_funcs.fnConvertToUncounted[vindex](result, env);

  return result;
}

void BespokeArray::ReleaseUncounted(ArrayData* ad) {
  assertx(!ad->uncountedCowCheck());
  auto const vindex = getVtableIndex(ad);
  g_layout_funcs.fnReleaseUncounted[vindex](ad);

  auto const bytes = g_layout_funcs.fnHeapSize[vindex](ad);
  auto const extra = uncountedAllocExtra(ad, ad->hasApcTv());
  FreeUncounted(reinterpret_cast<char*>(ad) - extra, bytes + extra);
}

//////////////////////////////////////////////////////////////////////////////

// ArrayData interface
NO_PROFILING
void BespokeArray::Release(ArrayData* ad) {
  g_layout_funcs.fnRelease[getVtableIndex(ad)](ad);
}
NO_PROFILING
bool BespokeArray::IsVectorData(const ArrayData* ad) {
  return g_layout_funcs.fnIsVectorData[getVtableIndex(ad)](ad);
}

// RO access
NO_PROFILING
TypedValue BespokeArray::NvGetInt(const ArrayData* ad, int64_t key) {
  return g_layout_funcs.fnNvGetInt[getVtableIndex(ad)](ad, key);
}
NO_PROFILING
TypedValue BespokeArray::NvGetStr(const ArrayData* ad, const StringData* key) {
  return g_layout_funcs.fnNvGetStr[getVtableIndex(ad)](ad, key);
}
NO_PROFILING
TypedValue BespokeArray::NvGetIntThrow(const ArrayData* ad, int64_t key) {
  return g_layout_funcs.fnNvGetIntThrow[getVtableIndex(ad)](ad, key);
}
NO_PROFILING
TypedValue BespokeArray::NvGetStrThrow(const ArrayData* ad, const StringData* key) {
  return g_layout_funcs.fnNvGetStrThrow[getVtableIndex(ad)](ad, key);
}
TypedValue BespokeArray::GetPosKey(const ArrayData* ad, ssize_t pos) {
  return g_layout_funcs.fnGetPosKey[getVtableIndex(ad)](ad, pos);
}
NO_PROFILING
TypedValue BespokeArray::GetPosVal(const ArrayData* ad, ssize_t pos) {
  return g_layout_funcs.fnGetPosVal[getVtableIndex(ad)](ad, pos);
}
NO_PROFILING
bool BespokeArray::ExistsInt(const ArrayData* ad, int64_t key) {
  return NvGetInt(ad, key).is_init();
}
NO_PROFILING
bool BespokeArray::ExistsStr(const ArrayData* ad, const StringData* key) {
  return NvGetStr(ad, key).is_init();
}

// iteration
NO_PROFILING
ssize_t BespokeArray::IterBegin(const ArrayData* ad) {
  return g_layout_funcs.fnIterBegin[getVtableIndex(ad)](ad);
}
NO_PROFILING
ssize_t BespokeArray::IterLast(const ArrayData* ad) {
  return g_layout_funcs.fnIterLast[getVtableIndex(ad)](ad);
}
NO_PROFILING
ssize_t BespokeArray::IterEnd(const ArrayData* ad) {
  return g_layout_funcs.fnIterEnd[getVtableIndex(ad)](ad);
}
NO_PROFILING
ssize_t BespokeArray::IterAdvance(const ArrayData* ad, ssize_t pos) {
  return g_layout_funcs.fnIterAdvance[getVtableIndex(ad)](ad, pos);
}
NO_PROFILING
ssize_t BespokeArray::IterRewind(const ArrayData* ad, ssize_t pos) {
  return g_layout_funcs.fnIterRewind[getVtableIndex(ad)](ad, pos);
}

// RW access
NO_PROFILING
arr_lval BespokeArray::LvalInt(ArrayData* ad, int64_t key) {
  return g_layout_funcs.fnLvalInt[getVtableIndex(ad)](ad, key);
}
NO_PROFILING
arr_lval BespokeArray::LvalStr(ArrayData* ad, StringData* key) {
  return g_layout_funcs.fnLvalStr[getVtableIndex(ad)](ad, key);
}
NO_PROFILING
tv_lval BespokeArray::ElemInt(
    tv_lval lval, int64_t key, bool throwOnMissing) {
  auto const ad = lval.val().parr;
  return g_layout_funcs.fnElemInt[getVtableIndex(ad)](lval, key, throwOnMissing);
}
NO_PROFILING
tv_lval BespokeArray::ElemStr(
    tv_lval lval, StringData* key, bool throwOnMissing) {
  auto const ad = lval.val().parr;
  return g_layout_funcs.fnElemStr[getVtableIndex(ad)](lval, key, throwOnMissing);
}

// insertion
NO_PROFILING
ArrayData* BespokeArray::SetIntMove(ArrayData* ad, int64_t key, TypedValue v) {
  return g_layout_funcs.fnSetIntMove[getVtableIndex(ad)](ad, key, v);
}
NO_PROFILING
ArrayData* BespokeArray::SetStrMove(ArrayData* ad, StringData* key, TypedValue v) {
  return g_layout_funcs.fnSetStrMove[getVtableIndex(ad)](ad, key, v);
}

// deletion
NO_PROFILING
ArrayData* BespokeArray::RemoveInt(ArrayData* ad, int64_t key) {
  return g_layout_funcs.fnRemoveInt[getVtableIndex(ad)](ad, key);
}
NO_PROFILING
ArrayData* BespokeArray::RemoveStr(ArrayData* ad, const StringData* key) {
  return g_layout_funcs.fnRemoveStr[getVtableIndex(ad)](ad, key);
}

// sorting
NO_PROFILING
ArrayData* BespokeArray::EscalateForSort(ArrayData* ad, SortFunction sf) {
  if (!isSortFamily(sf) && ad->isVecType()) return ad->toDict(true);
  assertx(!ad->empty());
  return g_layout_funcs.fnPreSort[getVtableIndex(ad)](ad, sf);
}
NO_PROFILING
ArrayData* BespokeArray::PostSort(ArrayData* ad, ArrayData* vad) {
  assertx(vad->isVanilla());
  if (ad->toDataType() != vad->toDataType()) return vad;
  assertx(vad->hasExactlyOneRef());
  return g_layout_funcs.fnPostSort[getVtableIndex(ad)](ad, vad);
}

// high-level ops
NO_PROFILING
ArrayData* BespokeArray::AppendMove(ArrayData* ad, TypedValue v) {
  return g_layout_funcs.fnAppendMove[getVtableIndex(ad)](ad, v);
}
NO_PROFILING
ArrayData* BespokeArray::Pop(ArrayData* ad, Variant& out) {
  return g_layout_funcs.fnPop[getVtableIndex(ad)](ad, out);
}
void BespokeArray::OnSetEvalScalar(ArrayData*) {
  always_assert(false);
}

// copies and conversions
ArrayData* BespokeArray::CopyStatic(const ArrayData*) {
  always_assert(false);
}
NO_PROFILING
ArrayData* BespokeArray::SetLegacyArray(ArrayData* ad, bool copy, bool legacy) {
  return g_layout_funcs.fnSetLegacyArray[getVtableIndex(ad)](ad, copy, legacy);
}

//////////////////////////////////////////////////////////////////////////////

}
