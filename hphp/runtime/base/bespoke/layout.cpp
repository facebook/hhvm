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
#include <folly/Optional.h>
#include <vector>

namespace HPHP { namespace bespoke {

//////////////////////////////////////////////////////////////////////////////

using namespace jit;
using namespace jit::irgen;

namespace {
std::atomic<size_t> s_layoutTableIndex;
std::array<Layout*, Layout::kMaxIndex.raw + 1> s_layoutTable;
std::atomic<bool> s_hierarchyFinal = false;

constexpr LayoutIndex kBespokeTopIndex = {0};
}

Layout::Layout(LayoutIndex index, std::string description, LayoutSet parents)
  : m_index(index)
  , m_description(std::move(description))
  , m_parents(std::move(parents))
{
  assertx(!s_hierarchyFinal.load(std::memory_order_acquire));
  assertx(s_layoutTable[m_index.raw] == nullptr);
  s_layoutTable[m_index.raw] = this;
  assertx(checkInvariants());
}

/*
 * A BFS implementation used to traverse the bespoke type lattice.
 */
struct Layout::BFSWalker {
  BFSWalker(bool upward, LayoutIndex initIdx)
    : m_workQ({initIdx})
    , m_upward(upward)
  {
    assertx(IMPLIES(!upward, s_hierarchyFinal.load(std::memory_order_acquire)));
  }

  template <typename C>
  BFSWalker(bool upward, C&& col)
    : m_workQ(col.cbegin(), col.cend())
    , m_upward(upward)
  {
    assertx(IMPLIES(!upward, s_hierarchyFinal.load(std::memory_order_acquire)));
  }

  /*
   * Return the next new node in the graph discovered by BFS, or folly::none if
   * the BFS has terminated.
   */
  folly::Optional<LayoutIndex> next() {
    while (!m_workQ.empty()) {
      auto const index = m_workQ.front();
      m_workQ.pop_front();
      if (m_processed.find(index) != m_processed.end()) continue;
      m_processed.insert(index);
      auto const layout = FromIndex(index);
      auto const nextSet = m_upward ? layout->m_parents : layout->m_children;
      for (auto const next : nextSet) {
        m_workQ.push_back(next);
      }
      return index;
    }
    return folly::none;
  }

  /*
   * Checks if the BFS has encountered a given node.
   */
  bool hasSeen(LayoutIndex index) const {
    return m_processed.find(index) != m_processed.end();
  }

  /*
   * Returns the full set of nodes seen during the BFS.
   */
  LayoutSet allSeen() const { return m_processed; }

private:
  LayoutSet m_processed;
  std::deque<LayoutIndex> m_workQ;
  bool m_upward;
};

/*
 * Computes whether the layout is a descendent of another layout. To do so, we
 * simply perform an upward BFS from the current layout until we encounter the
 * other layout or the BFS terminates.
 */
bool Layout::isDescendentOf(const Layout* other) const {
  BFSWalker walker(true, index());
  while (auto const idx = walker.next()) {
    if (idx == other->index()) return true;
  }
  return false;
}

/*
 * Compute the full set of ancestors by running upward DFS until termination.
 */
Layout::LayoutSet Layout::computeAncestors() const {
  BFSWalker walker(true, index());
  while (walker.next()) {}
  return walker.allSeen();
}

/*
 * Compute the full set of descendents by running downward DFS until
 * termination.
 */
Layout::LayoutSet Layout::computeDescendents() const {
  BFSWalker walker(false, index());
  while (walker.next()) {}
  return walker.allSeen();
}

/*
 * Computes the least upper bound or the greatest lower bound via BFS. Because
 * the edges in our graph represent the covering relation of the lattice, the
 * greatest upper bound is the first node found by an upward BFS on the two
 * layouts, and the least upper bound is the first node found by a downward BFS
 * on the two layouts.
 */
const Layout* Layout::nearestBound(bool upward, const Layout* other) const {
  BFSWalker walkerA(upward, index());
  BFSWalker walkerB(upward, other->index());
  while (true) {
    auto const idxA = walkerA.next();
    if (idxA && walkerB.hasSeen(*idxA)) return FromIndex(*idxA);
    auto const idxB = walkerB.next();
    if (idxB && walkerA.hasSeen(*idxB)) return FromIndex(*idxB);

    if (!idxA && !idxB) return nullptr;
  }
}

/*
 * A less efficient implementation of least upper and greatest lower bound that
 * does not assume uniqueness. This implementation works for general DAGs and
 * is used to validate that the DAG is a lattice. For a lattice, it always
 * agrees with nearestBound.
 */
const Layout* Layout::nearestBoundDebug(bool upward, const Layout* other) const {
  assertx(IMPLIES(!upward, s_hierarchyFinal.load(std::memory_order_acquire)));
  LayoutSet myClosure = upward ? computeAncestors() : computeDescendents();
  LayoutSet otherClosure =
    upward ? other->computeAncestors() : other->computeDescendents();
  LayoutSet common;
  std::set_intersection(myClosure.cbegin(), myClosure.cend(),
                        otherClosure.cbegin(), otherClosure.cend(),
                        std::inserter(common, common.end()));
  LayoutSet workQ(common);
  for (auto const item : workQ) {
    auto const layout = FromIndex(item);
    for (auto const rel : upward ? layout->m_parents : layout->m_children) {
      auto const iter = common.find(rel);
      if (iter == common.end()) continue;
      common.erase(iter);
    }
  }
  assertx(common.size() <= 1);
  if (common.empty()) return nullptr;
  auto const layout = FromIndex(*common.cbegin());
  assertx(upward ? isDescendentOf(layout) : layout->isDescendentOf(this));
  assertx(upward ? other->isDescendentOf(layout) : layout->isDescendentOf(other));
  return layout;
}

bool Layout::checkInvariants() const {
  if (!allowBespokeArrayLikes()) return true;

  // 0. Parents are valid.
  for (auto const DEBUG_ONLY parent : m_parents) {
    assertx(FromIndex(parent));
  }

  // 1. Indices are a topological ordering for the descendent graph.
  for (auto const DEBUG_ONLY parent : m_parents) {
    assertx(parent < index());
  }

  // 2. The parents provided are immediate parents (i.e. the descendent graph
  // is a covering relation).
  {
    LayoutSet grandparents;
    for (auto const parent : m_parents) {
      auto const layout = FromIndex(parent);
      std::copy(layout->m_parents.cbegin(), layout->m_parents.cend(),
                std::inserter(grandparents, grandparents.end()));
    }
    BFSWalker walker(true, grandparents);
    auto const all = walker.allSeen();
    LayoutSet inter;
    std::set_intersection(all.cbegin(), all.cend(),
                          m_parents.cbegin(), m_parents.cend(),
                          std::inserter(inter, inter.end()));
    assertx(inter.empty());
  }

  // 3. Least upper bound exists and is unique.
  for (size_t i = 0; i < index().raw; i++) {
    auto const other = s_layoutTable[i];
    if (!other) continue;
    auto const DEBUG_ONLY lub = nearestBoundDebug(true, other);
    assertx(lub);
    assertx(lub == other->nearestBoundDebug(true, this));
  }

  return true;
}

void Layout::FinalizeHierarchy() {
  assertx(allowBespokeArrayLikes());
  assertx(!s_hierarchyFinal.load(std::memory_order_acquire));
  for (size_t i = 0; i < s_layoutTableIndex; i++) {
    auto const layout = s_layoutTable[i];
    if (!layout) continue;
    assertx(layout->checkInvariants());
    for (auto const pIdx : layout->m_parents) {
      auto const parent = s_layoutTable[pIdx.raw];
      assertx(parent);
      parent->m_children.insert(layout->index());
    }
  }
  s_hierarchyFinal.store(true, std::memory_order_release);
}

bool Layout::operator<=(const Layout& other) const {
  if (!s_hierarchyFinal.load(std::memory_order_acquire)) {
    always_assert(index() == kBespokeTopIndex);
    always_assert(other.index() == kBespokeTopIndex);
    return true;
  }
  return isDescendentOf(&other);
}

const Layout* Layout::operator|(const Layout& other) const {
  if (!s_hierarchyFinal.load(std::memory_order_acquire)) {
    always_assert(index() == kBespokeTopIndex);
    always_assert(other.index() == kBespokeTopIndex);
    return this;
  }
  auto const bound = nearestBound(true, &other);
  assertx(bound == nearestBoundDebug(true, &other));
  always_assert(bound);
  return bound;
}

const Layout* Layout::operator&(const Layout& other) const {
  if (!s_hierarchyFinal.load(std::memory_order_acquire)) {
    always_assert(index() == kBespokeTopIndex);
    always_assert(other.index() == kBespokeTopIndex);
    return this;
  }
  auto const bound = nearestBound(false, &other);
  assertx(bound == nearestBoundDebug(false, &other));
  return bound;
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

struct Layout::Initializer {
  Initializer() {
    AbstractLayout::InitializeLayouts();
    LoggingArray::InitializeLayouts();
    MonotypeVec::InitializeLayouts();
    EmptyMonotypeDict::InitializeLayouts();
  }
};
Layout::Initializer Layout::s_initializer;

//////////////////////////////////////////////////////////////////////////////

SSATmp* Layout::emitGet(
    IRGS& env, SSATmp* arr, SSATmp* key, Block* taken) const {
  auto const val = gen(env, BespokeGet, TCell, arr, key);
  return gen(env, CheckType, TInitCell, taken, val);
}

SSATmp* Layout::emitElem(
    IRGS& env, SSATmp* arr, SSATmp* key, bool throwOnMissing) const {
  return gen(env, BespokeElem, TInitCell, arr, key, cns(env, throwOnMissing));
}

SSATmp* Layout::emitSet(
    IRGS& env, SSATmp* arr, SSATmp* key, SSATmp* val) const {
  return gen(env, BespokeSet, arr, key, val);
}

SSATmp* Layout::emitAppend(IRGS& env, SSATmp* arr, SSATmp* val) const {
  return gen(env, BespokeAppend, arr, val);
}

SSATmp* Layout::emitEscalateToVanilla(
    IRGS& env, SSATmp* arr, const char* reason) const {
  auto const str = cns(env, makeStaticString(reason));
  return gen(env, BespokeEscalateToVanilla, arr, str);
}

SSATmp* Layout::emitIterFirstPos(IRGS& env, SSATmp* arr) const {
  return gen(env, BespokeIterFirstPos, arr);
}

SSATmp* Layout::emitIterLastPos(IRGS& env, SSATmp* arr) const {
  return gen(env, BespokeIterLastPos, arr);
}

SSATmp* Layout::emitIterPos(IRGS& env, SSATmp* arr, SSATmp* idx) const {
  return idx;
}

SSATmp* Layout::emitIterAdvancePos(IRGS& env, SSATmp* arr, SSATmp* pos) const {
  return gen(env, BespokeIterAdvancePos, arr, pos);
}

SSATmp* Layout::emitIterElm(IRGS& env, SSATmp* arr, SSATmp* pos) const {
  return pos;
}

SSATmp* Layout::emitIterGetKey(IRGS& env, SSATmp* arr, SSATmp* elm) const {
  auto const type = arr->isA(TVec|TVArr) ? TInt : TInt|TStr;
  return gen(env, BespokeIterGetKey, type, arr, elm);
}

SSATmp* Layout::emitIterGetVal(IRGS& env, SSATmp* arr, SSATmp* elm) const {
  return gen(env, BespokeIterGetVal, TInitCell, arr, elm);
}

//////////////////////////////////////////////////////////////////////////////

AbstractLayout::AbstractLayout(LayoutIndex index,
                               std::string description,
                               LayoutSet parents)
  : Layout(index, std::move(description), std::move(parents))
{}

void AbstractLayout::InitializeLayouts() {
  auto const index = Layout::ReserveIndices(1);
  always_assert(index == kBespokeTopIndex);
  new AbstractLayout(index, "BespokeTop", {});
}

LayoutIndex AbstractLayout::GetBespokeTopIndex() {
  return kBespokeTopIndex;
}

ConcreteLayout::ConcreteLayout(LayoutIndex index,
                               std::string description,
                               const LayoutFunctions* vtable,
                               LayoutSet parents)
  : Layout(index, std::move(description), std::move(parents))
  , m_vtable(vtable)
{
  assertx(vtable);
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
  DEBUG_ONLY auto const index = BespokeArray::asBespoke(ad)->layoutIndex();
  DEBUG_ONLY auto const layout = BespokeLayout::FromIndex(index);
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

  auto const dt = et.valueDatatype;
  return ad->isDArray() || ad->isDictType()
    ? MakeMonotypeDictFromVanilla(ad, dt_modulo_persistence(dt), et.keyTypes)
    : MonotypeVec::MakeFromVanilla(ad, dt);
}
}

ArrayData* makeBespokeForTesting(ArrayData* ad, LoggingProfile* profile) {
  if (!jit::mcgen::retranslateAllEnabled()) {
    return bespoke::maybeMakeLoggingArray(ad, profile);
  }
  auto const mod = requestCount() % 3;
  if (mod == 1) return bespoke::maybeMakeLoggingArray(ad, profile);
  if (mod == 2) return bespoke::maybeMonoify(ad);
  return ad;
}

//////////////////////////////////////////////////////////////////////////////

}}
