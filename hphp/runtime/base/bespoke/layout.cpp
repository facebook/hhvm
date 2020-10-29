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

#include "hphp/runtime/base/bespoke/layout.h"

#include "hphp/runtime/base/bespoke/bespoke-top.h"
#include "hphp/runtime/base/bespoke/logging-array.h"
#include "hphp/runtime/base/bespoke/logging-profile.h"
#include "hphp/runtime/base/bespoke/monotype-dict.h"
#include "hphp/runtime/base/bespoke/monotype-vec.h"
#include "hphp/runtime/base/packed-array-defs.h"
#include "hphp/runtime/vm/jit/irgen.h"
#include "hphp/runtime/vm/jit/mcgen-translate.h"
#include "hphp/runtime/vm/jit/punt.h"
#include "hphp/util/trace.h"

#include <atomic>
#include <array>
#include <folly/lang/Bits.h>

namespace HPHP { namespace bespoke {

//////////////////////////////////////////////////////////////////////////////

using namespace jit;
using namespace jit::irgen;

namespace {
std::atomic<size_t> s_layoutTableIndex;
std::array<Layout*, Layout::kMaxIndex.raw + 1> s_layoutTable;
}

Layout::Layout(const std::string& description)
    : m_index(ReserveIndices(1)), m_description(description) {
  assertx(s_layoutTable[m_index.raw] == nullptr);
  s_layoutTable[m_index.raw] = this;
}

Layout::Layout(LayoutIndex index, const std::string& description)
    : m_index(index), m_description(description) {
  assertx(s_layoutTable[m_index.raw] == nullptr);
  s_layoutTable[m_index.raw] = this;
}

LayoutIndex Layout::ReserveIndices(size_t count) {
  assertx(folly::isPowTwo(count));
  auto const padded_count = 2 * count - 1;
  auto const base = s_layoutTableIndex.fetch_add(padded_count);
  always_assert(base + padded_count <= kMaxIndex.raw + 1);
  for (auto i = 0; i < padded_count; i++) {
    s_layoutTable[base + i] = nullptr;
  }
  auto const result = (base + count - 1) & ~(count - 1);
  assertx(result % count == 0);
  return {safe_cast<uint16_t>(result)};
}

const Layout* Layout::FromIndex(LayoutIndex index) {
  auto const layout = s_layoutTable[index.raw];
  assertx(layout != nullptr);
  assertx(layout->index() == index);
  return layout;
}

SSATmp* Layout::emitEscalateToVanilla(IRGS& env, SSATmp* arr,
                                      const char* reason) const {
  auto const data = BespokeLayoutData { nullptr };
  auto const str = cns(env, makeStaticString(reason));
  return gen(env, BespokeEscalateToVanilla, data, arr, str);
}

struct Layout::Initializer {
  Initializer() {
    assertx(s_layoutTableIndex.load(std::memory_order_relaxed) == 0);
    LoggingArray::InitializeLayouts();
    BespokeTop::InitializeLayouts();
    MonotypeVec::InitializeLayouts();
    EmptyMonotypeDict::InitializeLayouts();
  }
};
Layout::Initializer Layout::s_initializer;

//////////////////////////////////////////////////////////////////////////////

ConcreteLayout::ConcreteLayout(const std::string& description,
                               const LayoutFunctions* vtable = nullptr)
  : Layout(description)
  , m_vtable(vtable)
{
  assertx(vtable);
}

ConcreteLayout::ConcreteLayout(LayoutIndex index,
                               const std::string& description,
                               const LayoutFunctions* vtable = nullptr)
  : Layout(index, description)
  , m_vtable(vtable)
{
  assertx(vtable);
}

SSATmp* ConcreteLayout::emitGet(
    IRGS& env, SSATmp* arr, SSATmp* key, Block* taken) const {
  auto const data = BespokeLayoutData { this };
  return gen(env, BespokeGet, TCell, data, taken, arr, key);
}

SSATmp* ConcreteLayout::emitElem(
    IRGS& env, SSATmp* arr, SSATmp* key, bool throwOnMissing) const {
  auto const data = BespokeLayoutData { this };
  return gen(env, BespokeElem, TCell, data, arr, key, cns(env, throwOnMissing));
}

SSATmp* ConcreteLayout::emitSet(
    IRGS& env, SSATmp* arr, SSATmp* key, SSATmp* val) const {
  PUNT(unimpl_bespoke_emitSet);
}

SSATmp* ConcreteLayout::emitAppend(IRGS& env, SSATmp* arr, SSATmp* val) const {
  PUNT(unimpl_bespoke_emitAppend);
}

SSATmp* ConcreteLayout::emitEscalateToVanilla(IRGS& env, SSATmp* arr,
                                              const char* reason) const {
  auto const data = BespokeLayoutData { this };
  auto const str = cns(env, makeStaticString(reason));
  return gen(env, BespokeEscalateToVanilla, data, arr, str);
}

SSATmp* ConcreteLayout::emitIterFirstPos(IRGS& env, SSATmp* arr) const {
  auto const data = BespokeLayoutData { this };
  return gen(env, BespokeIterFirstPos, data, arr);
}

SSATmp* ConcreteLayout::emitIterLastPos(IRGS& env, SSATmp* arr) const {
  auto const data = BespokeLayoutData { this };
  return gen(env, BespokeIterLastPos, data, arr);
}

SSATmp* ConcreteLayout::emitIterPos(
    IRGS& env, SSATmp* arr, SSATmp* idx) const {
  PUNT(unimpl_bespoke_iterpos);
}

SSATmp* ConcreteLayout::emitIterAdvancePos(
    IRGS& env, SSATmp* arr, SSATmp* pos) const {
  auto const data = BespokeLayoutData { this };
  return gen(env, BespokeIterAdvancePos, data, arr, pos);
}

SSATmp* ConcreteLayout::emitIterElm(IRGS& env, SSATmp* arr, SSATmp* pos) const {
  return pos;
}

SSATmp* ConcreteLayout::emitIterGetKey(
    IRGS& env, SSATmp* arr, SSATmp* elm) const {
  auto const data = BespokeLayoutData { this };
  auto const retType = arr->isA(TVec|TVArr) ? TInt : TInt|TStr;
  return gen(env, BespokeIterGetKey, retType, data, arr, elm);
}

/**
 * This default implementation invokes the layout-specific GetPosVal method
 * without virtualization.
 */
SSATmp* ConcreteLayout::emitIterGetVal(
    IRGS& env, SSATmp* arr, SSATmp* elm) const {
  auto const data = BespokeLayoutData { this };
  return gen(env, BespokeIterGetVal, TCell, data, arr, elm);
}

const ConcreteLayout* ConcreteLayout::FromConcreteIndex(LayoutIndex index) {
  auto const layout = s_layoutTable[index.raw];
  assertx(layout != nullptr);
  assertx(layout->index() == index);
  assertx(layout->isConcrete());
  return reinterpret_cast<ConcreteLayout*>(layout);
}

//////////////////////////////////////////////////////////////////////////////

void logBespokeDispatch(const ArrayData* ad, const char* fn) {
  DEBUG_ONLY auto const sk = getSrcKey();
  DEBUG_ONLY auto const layout = BespokeArray::asBespoke(ad)->layout();
  TRACE_MOD(Trace::bespoke, 6, "Bespoke dispatch: %s: %s::%s\n",
            sk.getSymbol().data(), layout.describe().data(), fn);
}

namespace {
ArrayData* maybeMonoify(ArrayData* ad) {
  if (!ad->isVanilla() || ad->isKeysetType()) return ad;

  auto const et = EntryTypes::ForArray(ad);
  auto const monotype_keys =
    et.keyTypes == KeyTypes::Ints ||
    et.keyTypes == KeyTypes::Strings ||
    et.keyTypes == KeyTypes::StaticStrings ||
    et.keyTypes == KeyTypes::Empty;
  auto const monotype_vals =
    et.valueTypes == ValueTypes::Monotype ||
    et.valueTypes == ValueTypes::Empty;

  assertx(IMPLIES(ad->isVArray() || ad->isVecType(), monotype_keys));

  if (!(monotype_keys && monotype_vals)) {
    return ad;
  }

  SCOPE_EXIT { ad->decRefAndRelease(); };

  auto const legacy = ad->isLegacyArray();

  if (et.valueTypes == ValueTypes::Empty) {
    switch (ad->toDataType()) {
      case KindOfVArray: return EmptyMonotypeVec::GetVArray(legacy);
      case KindOfVec:    return EmptyMonotypeVec::GetVec(legacy);
      case KindOfDArray: return EmptyMonotypeDict::GetDArray(legacy);
      case KindOfDict:   return EmptyMonotypeDict::GetDict(legacy);
      default: always_assert(false);
    }
  }

  auto const dt = dt_modulo_persistence(et.valueDatatype);
  return ad->isDArray() || ad->isDictType()
    ? MakeMonotypeDictFromVanilla(ad, dt, et.keyTypes)
    : MonotypeVec::MakeFromVanilla(ad, dt);
}
}

ArrayData* makeBespokeForTesting(ArrayData* ad, LoggingProfile* profile) {
  if (!jit::mcgen::retranslateAllEnabled()) {
    return bespoke::maybeMakeLoggingArray(ad, profile);
  }
  auto const mod = requestCount() % 3;
  if (mod == 1) return bespoke::maybeMakeLoggingArray(ad);
  if (mod == 2) return bespoke::maybeMonoify(ad);
  return ad;
}

//////////////////////////////////////////////////////////////////////////////

}}
