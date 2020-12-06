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
#include "hphp/runtime/vm/jit/irgen-internal.h"
#include "hphp/runtime/vm/jit/mcgen-translate.h"
#include "hphp/runtime/vm/jit/punt.h"
#include "hphp/util/trace.h"

#include <atomic>
#include <array>
#include <folly/lang/Bits.h>
#include <folly/Optional.h>
#include <vector>
#include <sstream>

namespace HPHP { namespace bespoke {

//////////////////////////////////////////////////////////////////////////////

using namespace jit;
using namespace jit::irgen;

namespace {

std::atomic<size_t> s_topoIndex;
std::atomic<size_t> s_layoutTableIndex;
std::array<Layout*, Layout::kMaxIndex.raw + 1> s_layoutTable;
std::atomic<bool> s_hierarchyFinal = false;
std::mutex s_layoutCreationMutex;

constexpr LayoutIndex kBespokeTopIndex = {0};

bool isSingletonLayout(LayoutIndex index) {
  return index == EmptyMonotypeVecLayout::Index() ||
         index == EmptyMonotypeDictLayout::Index();
}

using Split = std::pair<uint16_t, std::vector<uint16_t>>;

folly::Optional<MaskAndCompare> computeSimpleMaskAndCompare(
    const std::vector<uint16_t>& liveVec,
    const std::vector<uint16_t>& deadVec) {
  uint16_t liveAgree = 0xffff;
  for (auto const live : liveVec) {
    auto const agree = ~(live ^ liveVec[0]);
    liveAgree &= agree;
  }

  const uint16_t base = liveVec[0] & liveAgree;
  for (auto const dead : deadVec) {
    if ((dead & liveAgree) == base) {
      return folly::none;
    }
  }

  return MaskAndCompare{base, liveAgree, 0};
}

folly::Optional<MaskAndCompare> computeXORMaskAndCompare(
    const std::vector<uint16_t>& liveVec,
    const std::vector<uint16_t>& deadVec) {
  std::vector<uint16_t> remainingLive(liveVec);
  std::vector<uint16_t> remainingDead(deadVec);

  // We process each bit from most significant to least significant,
  // determining the proper assignment for that bit in the XOR and CMP
  // values.
  //
  // For each bit, there three relevant scenarios:
  //
  // 1. All live layouts agree on this bit having value x. We should
  // XOR with x and CMP with 0.
  //
  // 2. Live layouts do not agree on the bit, but dead layouts agree on
  // it having value y. We should XOR with ~y and CMP with 1.
  //
  // 3. Live layouts do not agree on the bit, and neither do dead
  // layouts.  We must exclude this bit in the mask.
  //
  // After each action, we remove all dead layouts that are guaranteed
  // the fail our test and all live layouts that are guaranteed to pass
  // it.
  auto const recomputeLive = [&] () -> std::pair<uint16_t, uint16_t> {
    uint16_t liveAgree = 0xffff;
    for (auto const live : remainingLive) {
      auto const agree = ~(live ^ remainingLive[0]);
      liveAgree &= agree;
    }
    uint16_t deadAgree = 0xffff;
    for (auto const dead : remainingDead) {
      auto const agree = ~(dead ^ remainingDead[0]);
      deadAgree &= agree;
    }
    return {liveAgree, deadAgree};
  };

  uint16_t xorVal = 0;
  uint16_t cmpVal = 0;
  uint16_t andVal = 0xffff;
  auto [liveAgree, deadAgree] = recomputeLive();
  for (int i = 0; i < 16; i++) {
    // The bit currently being examined.
    const uint16_t mask = (1 << (15 - i));

    if (remainingDead.empty() && remainingLive.empty()) break;

    if (!remainingLive.empty() && (liveAgree & mask)) {
      xorVal |= remainingLive[0] & mask;
    } else if (!remainingDead.empty() && (deadAgree & mask)) {
      xorVal |= (~remainingDead[0]) & mask;
      cmpVal |= mask;
    } else if (remainingDead.empty()) {
      cmpVal |= mask;
    } else if (!remainingLive.empty()) {
      andVal ^= mask;
      // Nothing will be filtered out as we have not constrained our mask, so
      // reuse the agreement sets from the prior round.
      continue;
    }

    // The set of bits that have already been fixed and will not be masked
    // away. This is a prefix of the set of bits that will be fixed and is used
    // for determining which live and dead layouts must pass or fail the test,
    // regardless of the decisions made for future (less significant) bits.
    const uint16_t cmpMask = andVal & ~(mask - 1);

    remainingDead.erase(
      std::remove_if(remainingDead.begin(), remainingDead.end(),
        [&](auto v) { return ((v ^ xorVal) & cmpMask) > cmpVal; }),
      remainingDead.end());
    remainingLive.erase(
      std::remove_if(remainingLive.begin(), remainingLive.end(),
        [&](auto v) { return ((v ^ xorVal) & cmpMask) < cmpVal; }),
      remainingLive.end());

    auto const [liveAgreeNew, deadAgreeNew] = recomputeLive();
    liveAgree = liveAgreeNew;
    deadAgree = deadAgreeNew;
  }

  if (!remainingDead.empty() && !remainingLive.empty()) {
    return folly::none;
  }

  // If it came down to the last bit, adjust the comparison accordingly.
  if (!remainingDead.empty()) cmpVal--;

  return MaskAndCompare{xorVal, andVal, cmpVal};
}

}

Layout::Layout(LayoutIndex index, std::string description, LayoutSet parents)
  : m_index(index)
  , m_description(std::move(description))
  , m_parents(std::move(parents))
{
  std::lock_guard<std::mutex> lock(s_layoutCreationMutex);
  assertx(!s_hierarchyFinal.load(std::memory_order_acquire));
  assertx(s_layoutTable[m_index.raw] == nullptr);
  s_layoutTable[m_index.raw] = this;
  m_topoIndex = s_topoIndex++;
  assertx(checkInvariants());
}

/*
 * A BFS implementation used to traverse the bespoke type lattice.
 */
struct Layout::BFSWalker {
  BFSWalker(bool upward, LayoutIndex initIdx)
    : m_workQ({initIdx})
    , m_upward(upward)
  {}

  template <typename C>
  BFSWalker(bool upward, C&& col)
    : m_workQ(col.cbegin(), col.cend())
    , m_upward(upward)
  {}

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
 * Computes whether the layout is a descendant of another layout. To do so, we
 * simply perform an upward BFS from the current layout until we encounter the
 * other layout or the BFS terminates.
 */
bool Layout::isDescendantOfDebug(const Layout* other) const {
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
 * Compute the full set of descendants by running downward DFS until
 * termination.
 */
Layout::LayoutSet Layout::computeDescendants() const {
  BFSWalker walker(false, index());
  while (walker.next()) {}
  return walker.allSeen();
}

/*
 * A less efficient implementation of least upper and greatest lower bound that
 * does not assume uniqueness. This implementation works for general DAGs and
 * is used to validate that the DAG is a lattice.
 */
const Layout* Layout::nearestBoundDebug(bool upward, const Layout* other) const {
  assertx(IMPLIES(!upward, s_hierarchyFinal.load(std::memory_order_acquire)));
  LayoutSet myClosure = upward ? computeAncestors() : computeDescendants();
  LayoutSet otherClosure =
    upward ? other->computeAncestors() : other->computeDescendants();
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

  SCOPE_ASSERT_DETAIL("bespoke::Layout::nearestBoundDebug") {
    std::string result = folly::sformat(
        "Found multiple nodes when computing {} bound for {} and {}:\n",
        upward ? "an upper" : "a lower", describe(), other->describe());
    for (auto const item : common) {
      result += folly::sformat("  {}\n", Layout::FromIndex(item)->describe());
    }
    return result;
  };

  assertx(common.size() <= 1);
  if (common.empty()) return nullptr;
  auto const layout = FromIndex(*common.cbegin());
  assertx(upward ? isDescendantOfDebug(layout)
                 : layout->isDescendantOfDebug(this));
  assertx(upward ? other->isDescendantOfDebug(layout)
                 : layout->isDescendantOfDebug(other));
  return layout;
}

bool Layout::checkInvariants() const {
  if (!allowBespokeArrayLikes()) return true;

  // 0. Parents are valid.
  for (auto const DEBUG_ONLY parent : m_parents) {
    auto const DEBUG_ONLY layout = FromIndex(parent);
    assertx(layout);
    assertx(layout->m_topoIndex < m_topoIndex);
  }

  // 1. The parents provided are immediate parents (i.e. the descendant graph
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

  // 2. Least upper bound exists and is unique.
  for (size_t i = 0; i < index().raw; i++) {
    auto const other = s_layoutTable[i];
    if (!other) continue;
    auto const DEBUG_ONLY lub = nearestBoundDebug(true, other);
    assertx(lub);
    assertx(lub == other->nearestBoundDebug(true, this));
  }

  return true;
}

struct Layout::DescendantOrdering {
  bool operator()(const Layout* a, const Layout* b) const {
    return a->m_topoIndex < b->m_topoIndex;
  }
};

struct Layout::AncestorOrdering {
  bool operator()(const Layout* a, const Layout* b) const {
    return a->m_topoIndex > b->m_topoIndex;
  }
};

void Layout::FinalizeHierarchy() {
  assertx(allowBespokeArrayLikes());
  assertx(!s_hierarchyFinal.load(std::memory_order_acquire));
  std::vector<Layout*> allLayouts;
  for (size_t i = 0; i < s_layoutTableIndex; i++) {
    auto const layout = s_layoutTable[i];
    if (!layout) continue;
    allLayouts.push_back(layout);
    assertx(layout->checkInvariants());
    for (auto const pIdx : layout->m_parents) {
      auto const parent = s_layoutTable[pIdx.raw];
      assertx(parent);
      parent->m_children.insert(layout->index());
    }
  }

  // TODO(mcolavita): implement these by merging in topological order instead
  // of repeated BFS
  for (size_t i = 0; i < s_layoutTableIndex; i++) {
    auto layout = s_layoutTable[i];
    if (!layout) continue;
    auto const descendants = layout->computeDescendants();
    std::transform(
      descendants.begin(), descendants.end(),
      std::back_inserter(layout->m_descendants),
      [&] (LayoutIndex i) { return s_layoutTable[i.raw]; }
    );
    auto const ancestors = layout->computeAncestors();
    std::transform(
      ancestors.begin(), ancestors.end(),
      std::back_inserter(layout->m_ancestors),
      [&] (LayoutIndex i) { return s_layoutTable[i.raw]; }
    );

    std::sort(layout->m_descendants.begin(), layout->m_descendants.end(),
              DescendantOrdering{});
    std::sort(layout->m_ancestors.begin(), layout->m_ancestors.end(),
              AncestorOrdering{});
  }

  // Compute mask and compare sets in topological order so that they can use
  // their children's masks if necessary.
  std::sort(allLayouts.begin(), allLayouts.end(), AncestorOrdering{});
  for (auto const layout : allLayouts) {
    layout->m_maskAndCompare = layout->computeMaskAndCompare();
  }

  s_hierarchyFinal.store(true, std::memory_order_release);
}

bool Layout::operator<=(const Layout& other) const {
  assertx(s_hierarchyFinal.load(std::memory_order_acquire));
  auto const res = std::binary_search(m_ancestors.begin(), m_ancestors.end(),
                                      &other, AncestorOrdering{});
  assertx(isDescendantOfDebug(&other) == res);
  return res;
}

const Layout* Layout::operator|(const Layout& other) const {
  assertx(s_hierarchyFinal.load(std::memory_order_acquire));
  auto lIter = m_ancestors.cbegin();
  auto rIter = other.m_ancestors.cbegin();
  auto const lt = AncestorOrdering{};
  while (lIter != m_ancestors.cend() && rIter != other.m_ancestors.cend()) {
    if (*lIter == *rIter) {
      assertx(*lIter == nearestBoundDebug(true, &other));
      return *lIter;
    }
    if (lt(*lIter, *rIter)) {
      lIter++;
    } else {
      rIter++;
    }
  }
  not_reached();
}

const Layout* Layout::operator&(const Layout& other) const {
  assertx(s_hierarchyFinal.load(std::memory_order_acquire));
  auto lIter = m_descendants.cbegin();
  auto rIter = other.m_descendants.cbegin();
  auto const lt = DescendantOrdering{};
  while (lIter != m_descendants.cend() && rIter != other.m_descendants.cend()) {
    if (*lIter == *rIter) {
      assertx(*lIter == nearestBoundDebug(false, &other));
      return *lIter;
    }
    if (lt(*lIter, *rIter)) {
      lIter++;
    } else {
      rIter++;
    }
  }
  assertx(!nearestBoundDebug(false, &other));
  return nullptr;
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

std::string Layout::dumpAllLayouts() {
  std::ostringstream ss;

  for (size_t i = 0; i < s_layoutTableIndex; i++) {
    auto const layout = s_layoutTable[i];
    if (!layout) continue;

    ss << layout->dumpInformation();
  }

  return ss.str();
}

std::string Layout::dumpInformation() const {
  assertx(s_hierarchyFinal.load(std::memory_order_acquire));

  auto const concreteDesc = [&](const Layout* layout) {
    return layout->isConcrete() ? "Concrete" : "Abstract";
  };

  std::ostringstream ss;
  ss << folly::format("{:04x}: {} [{}]\n", m_index.raw, describe(),
                       concreteDesc(this));

  ss << folly::format("  Type test:\n");
  ss << folly::format("    XOR {:04x}\n", m_maskAndCompare.xorVal);
  ss << folly::format("    AND {:04x}\n", m_maskAndCompare.andVal);
  ss << folly::format("    CMP {:04x}\n", m_maskAndCompare.cmpVal);

  ss << folly::format("  Ancestors:\n");
  for (auto const ancestor : m_ancestors) {
    ss << folly::format("  - {:04x}: {} [{}]\n",
                        ancestor->m_index.raw, ancestor->describe(),
                        concreteDesc(ancestor));
  }

  ss << folly::format("  Descendants:\n");
  for (auto const descendant : m_descendants) {
    ss << folly::format("  - {:04x}: {} [{}]\n",
                        descendant->m_index.raw, descendant->describe(),
                        concreteDesc(descendant));
  }

  ss << "\n";

  return ss.str();
}

MaskAndCompare Layout::computeMaskAndCompare() const {
  FTRACE_MOD(Trace::bespoke, 1, "Try: {}\n", describe());

  // The set of all concrete layouts that descend from the layout.
  std::vector<uint16_t> liveVec;
  for(auto const layout : m_descendants) {
    if (!layout->isConcrete()) continue;
    liveVec.push_back(layout->m_index.raw);
  }

  assertx(!liveVec.empty());
  if (liveVec.size() == 1) return {MaskAndCompare::fullCompare(liveVec[0])};
  std::sort(liveVec.begin(), liveVec.end());

  // The set of all possible concrete layouts.
  std::vector<uint16_t> allConcrete;
  for (size_t i = 0; i < s_layoutTableIndex; i++) {
    auto const layout = s_layoutTable[i];
    if (!layout) continue;
    if (!layout->isConcrete()) continue;
    allConcrete.push_back(layout->m_index.raw);
  }

  // The set of all concrete layouts that do *not* descend from this layout.
  std::vector<uint16_t> deadVec;
  std::set_difference(
    allConcrete.cbegin(), allConcrete.cend(),
    liveVec.cbegin(), liveVec.cend(),
    std::back_inserter(deadVec)
  );

  auto const check = [&] () -> MaskAndCompare {
    // 1. Attempt to find a trivial mask to cover.
    if (auto const result = computeSimpleMaskAndCompare(liveVec, deadVec)) {
      return *result;
    }

    // 2. Attempt to find a single mask to cover.
    if (auto const result = computeXORMaskAndCompare(liveVec, deadVec)) {
      return *result;
    }

    // 3. Give up.
    SCOPE_ASSERT_DETAIL("bespoke::Layout::computeMaskAndCompare") {
      std::string ret = folly::sformat("{:04x}: {}\n", m_index.raw, describe());
      ret += folly::sformat("  Live:\n");
      for (auto const live : liveVec) {
        auto const layout = s_layoutTable[live];
        ret += folly::sformat("  - {:04x}: {}\n", live, layout->describe());
      }
      ret += folly::sformat("  Dead:\n");
      for (auto const dead : deadVec) {
        auto const layout = s_layoutTable[dead];
        ret += folly::sformat("  - {:04x}: {}\n", dead, layout->describe());
      }
      return ret;
    };
    always_assert(false);
  }();

  if (debug) {
    // The check should pass on live values and fail on dead values.
    for (auto const live : liveVec) always_assert(check.accepts(live));
    for (auto const dead : deadVec) always_assert(!check.accepts(dead));
  }

  return check;
}

MaskAndCompare Layout::maskAndCompare() const {
  assertx(s_hierarchyFinal.load(std::memory_order_acquire));
  return m_maskAndCompare;
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

namespace {
ArrayData* maybeMonoifyForTesting(ArrayData* ad, LoggingProfile* profile) {
  assertx(profile->data);
  auto& profileMonotype = profile->data->staticMonotypeArray;
  auto const mad = profileMonotype.load(std::memory_order_relaxed);
  if (mad) return mad;

  auto const result = maybeMonoify(ad);
  if (!result) return ad;
  ad->decRefAndRelease();

  // We should cache a staticMonotypeArray iff this profile is for a static
  // array constructor or static prop - i.e. iff staticLoggingArray is set.
  if (!profile->data->staticLoggingArray) return result;

  ArrayData* current = nullptr;
  if (profileMonotype.compare_exchange_strong(current, result)) {
    return result;
  }
  RO::EvalLowStaticArrays ? low_free(result) : uncounted_free(result);
  return current;
}
}

void logBespokeDispatch(const ArrayData* ad, const char* fn) {
  DEBUG_ONLY auto const sk = getSrcKey();
  DEBUG_ONLY auto const index = BespokeArray::asBespoke(ad)->layoutIndex();
  DEBUG_ONLY auto const layout = Layout::FromIndex(index);
  TRACE_MOD(Trace::bespoke, 6, "Bespoke dispatch: %s: %s::%s\n",
            sk.getSymbol().data(), layout->describe().data(), fn);
}

BespokeArray* maybeMonoify(ArrayData* ad) {
  if (!ad->isVanilla() || ad->isKeysetType()) return nullptr;

  auto const et = EntryTypes::ForArray(ad);
  if (!et.isMonotypeState()) return nullptr;

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
    ? MakeMonotypeDictFromVanilla(ad, dt, et.keyTypes)
    : MonotypeVec::MakeFromVanilla(ad, dt);
}

ArrayData* makeBespokeForTesting(ArrayData* ad, LoggingProfile* profile) {
  if (!jit::mcgen::retranslateAllEnabled()) {
    return bespoke::maybeMakeLoggingArray(ad, profile);
  }
  auto const mod = requestCount() % 3;
  if (mod == 1) return bespoke::maybeMakeLoggingArray(ad, profile);
  if (mod == 2) return bespoke::maybeMonoifyForTesting(ad, profile);
  return ad;
}

//////////////////////////////////////////////////////////////////////////////

}}
