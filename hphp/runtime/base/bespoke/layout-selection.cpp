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

#include "hphp/runtime/base/bespoke/layout-selection.h"

#include "hphp/runtime/base/bespoke/key-coloring.h"
#include "hphp/runtime/base/bespoke/layout.h"
#include "hphp/runtime/base/bespoke/monotype-dict.h"
#include "hphp/runtime/base/bespoke/monotype-vec.h"
#include "hphp/runtime/base/bespoke/struct-dict.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/util/union-find.h"

namespace HPHP { namespace bespoke {

TRACE_SET_MOD(bespoke);

//////////////////////////////////////////////////////////////////////////////

namespace {

//////////////////////////////////////////////////////////////////////////////

struct StructGroup {
  StructGroup(std::vector<const LoggingProfile*>&& profiles, double weight)
    : profiles(std::move(profiles))
    , weight(weight)
  {}

  std::vector<const LoggingProfile*> profiles;
  double weight;
  const StructLayout* layout;
};

//////////////////////////////////////////////////////////////////////////////

using jit::ArrayLayout;

// This type represents a decision of type T that we make based on profiling.
// It covers a fraction `coverage` of the profiled distribution.
template <typename T>
struct Decision {
  T value;
  double coverage;
};

uint64_t load(const std::atomic<uint64_t>& x) {
  return x.load(std::memory_order_relaxed);
}

double probabilityOfEscalation(const LoggingProfile& profile) {
  auto const logging = load(profile.data->loggingArraysEmitted);
  if (!logging) return 1.0;

  uint64_t escalated = 0;
  for (auto const& it : profile.data->events) {
    auto const op = getEventArrayOp(it.first);
    if (op == ArrayOp::EscalateToVanilla) escalated += it.second;
  }
  return 1.0 * std::min(escalated, logging) / logging;
}

double probabilityOfSampled(const SinkProfile& profile) {
  if (profile.data->sources.empty()) return 0.0;
  auto const sampled = load(profile.data->sampledCount);
  auto const unsampled = load(profile.data->unsampledCount);
  return 1.0 * sampled / (sampled + unsampled);
}

//////////////////////////////////////////////////////////////////////////////
// Monotype helpers

// Returns KeyTypes::Any if there's no good key type to specialize on.
Decision<KeyTypes> selectKeyType(const SinkProfile& profile, double p_cutoff) {
  assertx(profile.data);

  auto const empty = load(profile.data->keyCounts[int(KeyTypes::Empty)]);
  auto const ints  = load(profile.data->keyCounts[int(KeyTypes::Ints)]);
  auto const strs  = load(profile.data->keyCounts[int(KeyTypes::Strings)]);
  auto const sstrs = load(profile.data->keyCounts[int(KeyTypes::StaticStrings)]);
  auto const any   = load(profile.data->keyCounts[int(KeyTypes::Any)]);

  auto const total = empty + ints + strs + sstrs + any;
  if (!total) return {KeyTypes::Any, 1.0};

  auto const p_ints  = 1.0 * (empty + ints) / total;
  auto const p_strs  = 1.0 * (empty + strs + sstrs) / total;
  auto const p_sstrs = 1.0 * (empty + sstrs) / total;

  if (p_sstrs >= p_cutoff) return {KeyTypes::StaticStrings, p_sstrs};
  if (p_strs >= p_cutoff)  return {KeyTypes::Strings, p_strs};
  if (p_ints >= p_cutoff)  return {KeyTypes::Ints, p_ints};
  return {KeyTypes::Any, 1.0};
}

// Returns kKindOfUninit if we should specialize on "monotype of unknown type".
// Returns kInvalidDataType if we should not specialize this sink on a monotype.
Decision<DataType> selectValType(const SinkProfile& profile, double p_cutoff) {
  assertx(profile.data);

  auto const empty = load(profile.data->valCounts[SinkProfile::kNoValTypes]);
  auto const any   = load(profile.data->valCounts[SinkProfile::kAnyValType]);

  uint64_t total = empty + any;
  uint64_t max_count = 0;
  auto max_index = safe_cast<int>(SinkProfile::kNumValTypes);

  static_assert(SinkProfile::kNoValTypes == SinkProfile::kNumValTypes - 2);
  static_assert(SinkProfile::kAnyValType == SinkProfile::kNumValTypes - 1);
  for (auto i = 0; i < SinkProfile::kNoValTypes; i++) {
    auto const count = load(profile.data->valCounts[i]);
    total += count;
    if (count > max_count) {
      max_count = count;
      max_index = i;
    }
  }

  if (!total) return {kInvalidDataType, 1.0};

  auto const p_empty = 1.0 * empty / total;
  auto const p_max   = 1.0 * (empty + max_count) / total;
  auto const p_mono  = 1.0 * (total - any) / total;
  auto const max = safe_cast<DataType>(max_index + kMinDataType);

  // TODO(kshaunak): We may want to specialize on empty in the future.
  if (p_empty >= p_cutoff) return {KindOfUninit, p_mono};
  if (p_max >= p_cutoff)   return {max, p_max};
  if (p_mono >= p_cutoff)  return {KindOfUninit, p_mono};
  return {kInvalidDataType, 1.0};
}

//////////////////////////////////////////////////////////////////////////////
// Struct helpers

using KeyFrequencies = folly::F14FastMap<const StringData*, int64_t>;

struct StructAnalysis {
  std::vector<KeyOrder> key_orders;
  std::vector<const SinkProfile*> merge_sinks;
  UnionFind<const LoggingProfile*> union_find;
};

struct StructAnalysisResult {
  folly::F14FastMap<const LoggingProfile*, const StructLayout*> sources;
  folly::F14FastMap<const SinkProfile*, Decision<ArrayLayout>> sinks;
};

double keyCoverageThreshold() {
  return RO::EvalBespokeStructDictKeyCoverageThreshold / 100;
}

template <typename T, typename I, typename F>
void mergeValid(I&& begin, I&& end, UnionFind<T>& uf, F&& elem) {
  auto iter = begin;
  while (iter != end && !uf.lookup(elem(*iter))) iter++;
  if (iter == end) return;
  auto const& root = elem(*iter);
  for (; iter!= end; iter++) {
    auto const& val = elem(*iter);
    if (uf.lookup(val)) uf.merge(root, val);
  }
}

// Sort keys in KeyOrder by descending order of frequency.
KeyOrder sortKeyOrder(const KeyOrder& ko, const KeyFrequencies& keys) {
  if (ko.empty() || !ko.valid()) return ko;
  auto const frequency = [&](LowStringPtr key) {
    auto const it = keys.find(key.get());
    return it == keys.end() ? 0 : it->second;
  };
  KeyOrder::KeyOrderData sorted;
  for (auto const key : ko) {
    sorted.push_back(key);
  }
  std::sort(sorted.begin(), sorted.end(),
            [&](auto a, auto b) { return frequency(a) > frequency(b); });
  return KeyOrder::Make(sorted);
}

using KeyCountMap = folly::F14FastMap<const StringData*, size_t>;
using KeyOrderFrequencyList = std::vector<std::pair<KeyOrder, size_t>>;

// Returns a map from keys to counts, indicating a conservative approximation
// of the number of logged arrays containing (the static version of) each key.
// We miss keys that are in uncounted arrays or set via a non-static value
KeyCountMap arraysContainingKeys(
    const std::vector<const LoggingProfile*>& profiles, size_t bound) {
  auto arrayCounts = KeyCountMap();
  for (auto const& profile : profiles) {
    // Count ConstructStr/SetStr keys
    for (auto const& it : profile->data->events) {
      auto const op = getEventArrayOp(it.first);
      if (op == ArrayOp::ConstructStr || op == ArrayOp::SetStr) {
        auto const key = getEventStrKey(it.first);
        if (key != nullptr) {
          auto& count = arrayCounts[key.get()];
          count = std::min(count + it.second, bound);
        }
      }
    }

    // Count static keys
    auto const ad = profile->data->staticSampledArray;
    if (!ad) continue;
    assertx(ad->isVanillaDict());
    MixedArray::IterateKV(
      MixedArray::asMixed(ad),
      [&](auto k, auto) {
        if (!tvIsString(k)) return;
        auto& count = arrayCounts[val(k).pstr];
        count = std::min(
          size_t{count + load(profile->data->loggingArraysEmitted)}, bound);
      }
    );
  }
  return arrayCounts;
}

size_t countArrays(const std::vector<const LoggingProfile*>& profiles) {
  return std::accumulate(
    profiles.begin(),
    profiles.end(),
    size_t{0},
    [](auto const& a, auto const& b) {
      return a + load(b->data->loggingArraysEmitted);
    }
  );
}

/*
 * Returns a map indicating, for each key in the KeyOrderMap, how many key
 * order instances would be invalidated by removing it.
 */
KeyCountMap countKeyInstances(const KeyOrderFrequencyList& frequencyList) {
  auto keyInstances = KeyCountMap{};
  for (auto const& [keyOrder, count] : frequencyList) {
    for (auto const& key : keyOrder) {
      keyInstances[key] += count;
    }
  }

  return keyInstances;
}

KeyOrder pruneKeyOrder(
    const std::vector<const LoggingProfile*>& profiles, double cutoff) {
  auto const keyOrderMap = [&] {
    auto kom = KeyOrderMap();
    for (auto const profile : profiles) {
      mergeKeyOrderMap(kom, profile->data->keyOrders);
    }
    return kom;
  }();

  auto workingSet = KeyOrderFrequencyList();
  workingSet.reserve(keyOrderMap.size());
  for (auto const& [keyOrder, count] : keyOrderMap) {
    workingSet.emplace_back(keyOrder, count);
  }

  auto const sumCounts = [](auto begin, auto end) {
    return std::accumulate(
      begin, end,
      size_t{0},
      [](auto const& a, auto const& b) { return a + b.second; }
    );
  };

  // Compute the total number of key order instances we begin with.
  auto const totalOrders = sumCounts(workingSet.begin(), workingSet.end());
  auto const totalArrays = countArrays(profiles);
  auto const arraysContainingKey = arraysContainingKeys(profiles, totalArrays);

  // Immediately prune invalid key orders.
  auto acceptedOrders = totalOrders;
  auto acceptedArrays = totalArrays;

  {
    auto const removeAt = std::partition(
      workingSet.begin(), workingSet.end(),
      [&](auto const& a) { return a.first.valid(); }
    );
    auto const discardedOrders = sumCounts(removeAt, workingSet.end());
    acceptedOrders -= discardedOrders;
    workingSet.erase(removeAt, workingSet.end());
    assertx(acceptedOrders == sumCounts(workingSet.begin(), workingSet.end()));

    // Conservatively invalidate the same number of arrays.
    if (discardedOrders > acceptedArrays) return KeyOrder::MakeInvalid();
    acceptedArrays -= discardedOrders;

    FTRACE(2, "Prune invalid. Remain: ({} / {} ko), ({} / {} arrays)\n",
           acceptedOrders, totalOrders, acceptedArrays, totalArrays);
  }

  // If we're already below our cutoff, abort.
  if (acceptedOrders < totalOrders * cutoff ||
      acceptedArrays < totalArrays * cutoff) {
    return KeyOrder::MakeInvalid();
  }

  // Greedily remove the key which invalidates the least number of key order
  // instances. Stop when doing so again would cross the cutoff.
  auto keyInstances = countKeyInstances(workingSet);
  while (!keyInstances.empty()) {
    auto const keyAndCount = std::min_element(
      keyInstances.cbegin(), keyInstances.cend(),
      [](auto const& a, auto const& b) { return a.second < b.second; }
    );

    FTRACE(2, "Suggest prune key \"{}\".\n", keyAndCount->first);

    {
      acceptedOrders -= keyAndCount->second;
      auto const iter = arraysContainingKey.find(keyAndCount->first);
      // If we do not have any arrays registered as containing the key, then it
      // must be a key added during an uncounted APC logging array
      // construction. It should therefore be in all operation key orders, so
      // there's no need to pessimize here.
      auto const attributed =
        iter == arraysContainingKey.end() ? 0 : iter->second;
      if (attributed > acceptedArrays) break;
      acceptedArrays -= attributed;
      if (acceptedOrders < totalOrders * cutoff ||
          acceptedArrays < totalArrays * cutoff) {
        break;
      }
    }

    FTRACE(2, "Prune key \"{}\". Remain: ({} / {} ko), ({} / {} arrays)\n",
           keyAndCount->first, acceptedOrders, totalOrders,
           acceptedArrays, totalArrays);

    // Remove all key orders invalidated by this removal, and update the
    // instance counts.
    auto removeAt = std::partition(
      workingSet.begin(), workingSet.end(),
      [&](auto const& a) { return !a.first.contains(keyAndCount->first); }
    );
    std::for_each(
      removeAt, workingSet.end(),
      [&](auto const& pair) {
        for (auto const& key : pair.first) {
          auto const iter = keyInstances.find(key);
          assertx(iter != keyInstances.end());
          iter->second -= pair.second;
          if (iter->second == 0) {
            keyInstances.erase(iter);
          }
        }
      }
    );

    workingSet.erase(removeAt, workingSet.end());

    assertx(acceptedOrders == sumCounts(workingSet.begin(), workingSet.end()));
    assertx(keyInstances == countKeyInstances(workingSet));
  }

  // Assemble the final pruned key order.
  auto prunedResult = KeyOrderMap();
  for (auto const& [keyOrder, count] : workingSet) {
    prunedResult.emplace(keyOrder, count);
  }

  auto const result = collectKeyOrder(prunedResult);
  return result;
}

// Returns true if we treat the given sink as a "merge point" and union the
// sets of sinks that are incident to that merge. These points are also the
// only ones at which we'll JIT struct access code.
bool mergeStructsAtSink(const SinkProfile& profile, const StructAnalysis& sa) {
  // 1. We must have sufficient LoggingArray coverage for the given sink.
  auto const p_cutoff = RO::EvalBespokeArraySinkSpecializationThreshold / 100;
  auto const p_sampled = probabilityOfSampled(profile);
  if (p_sampled < p_cutoff) return false;

  // 2. All sources must be struct-like sources.
  KeyOrderMap kom;
  for (auto const& pair : profile.data->sources) {
    auto const index = sa.union_find.lookup(pair.first);
    if (!index) return false;
    kom.emplace(sa.key_orders[*index], 0).first->second.value++;
  }

  // 3. We must be able to construct a struct layout that covers all sources.
  auto const ko = collectKeyOrder(kom);
  if (ko.empty() || !ko.valid()) return false;

  // 4. We must not be able to split sources into two disjoint struct layouts.
  UnionFind<const StringData*> uf;
  for (auto const key : ko) {
    uf.insert(key);
  }
  for (auto const& pair : kom) {
    if (pair.first.empty()) continue;
    auto const root = *pair.first.begin();
    for (auto const key : pair.first) {
      uf.merge(root, key);
    }
  }
  return uf.countGroups() == 1;
}

void initStructAnalysis(const LoggingProfile& profile, StructAnalysis& sa) {
  // We only use a struct layout for a dict/darray array sources. We don't
  // include casts here because non-trivial casts (from vec/varray/keyset
  // dict/darray) will rarely produce useful struct-like arrays.
  auto const type_okay = [&]{
    switch (profile.key.locationType) {
      case LocationType::APCKey:
      case LocationType::Runtime:
        return true;
      case LocationType::InstanceProperty:
      case LocationType::StaticProperty:
      case LocationType::SrcKey:
        auto const vad = profile.data->staticSampledArray;
        if (vad != nullptr && vad->isDictType()) return true;
        auto const op = profile.key.op();
        return op == OpNewDictArray || op == OpNewStructDict;
    }
    always_assert(false);
  }();

  // Add a node for this source to our union-find table if the type matches
  // and there's a struct layout that's a good match for this source alone.
  if (!type_okay || profile.data->keyOrders.empty()) return;
  auto const threshold = keyCoverageThreshold();
  auto const ko = pruneKeyOrder({&profile}, threshold);
  if (!ko.valid()) return;
  DEBUG_ONLY auto const index = sa.union_find.insert(&profile);
  assertx(index == sa.key_orders.size());
  sa.key_orders.push_back(ko);
}

void updateStructAnalysis(const SinkProfile& profile, StructAnalysis& sa) {
  auto const& sources = profile.data->sources;
  if (sources.empty() || !mergeStructsAtSink(profile, sa)) return;
  mergeValid(sources.begin(), sources.end(), sa.union_find,
             [&](auto const& x) { return x.first; });
  sa.merge_sinks.push_back(&profile);
}

StructAnalysisResult finishStructAnalysis(StructAnalysis& sa) {
  auto const p_cutoff = RO::EvalBespokeArraySourceSpecializationThreshold / 100;
  auto groups = std::vector<StructGroup>{};

  // Sort groups by weight to preferentially create hot struct layouts.
  // We also filter out groups that are likely to need escalation here.
  sa.union_find.forEachGroup([&](auto& group) {
    double weight = 0;
    double p_escalated = 0;
    for (auto const source : group) {
      auto const source_weight = source->getProfileWeight();
      weight += source_weight;
      p_escalated += source_weight * probabilityOfEscalation(*source);
    }
    if (weight > 0 && !(p_escalated / weight > 1 - p_cutoff)) {
      groups.emplace_back(std::move(group), weight);
    }
  });
  std::sort(groups.begin(), groups.end(),
            [](auto const& a, auto const& b) { return a.weight > b.weight; });

  using LayoutWeightMap = folly::F14FastMap<const StructLayout*, double>;
  auto layoutWeights = LayoutWeightMap{};

  // Create potential layout set.
  for (auto& group : groups) {
    KeyFrequencies keys;
    for (auto const source : group.profiles) {
      auto const multiplier = source->getSampleCountMultiplier();
      for (auto const& it : source->data->events) {
        auto const key = getEventStrKey(it.first);
        if (key != nullptr) keys[key.get()] += it.second * multiplier;
      }
    }
    auto const threshold = keyCoverageThreshold();
    auto const groupKO =
      sortKeyOrder(pruneKeyOrder(group.profiles, threshold), keys);
    auto const layout = StructLayout::GetLayout(groupKO, true);
    group.layout = layout;
    if (layout) layoutWeights[layout] += group.weight;
  }

  auto layoutVector =
    LayoutWeightVector(layoutWeights.begin(), layoutWeights.end());

  // Find a colorable subset of the selected layouts.
  auto const [coloringEnd, coloring] = findKeyColoring(layoutVector);
  if (!coloring) return {};

  // Remove groups with missing or discarded StructLayouts.
  {
    auto discarded = folly::F14FastSet<const StructLayout*>();
    std::transform(
      coloringEnd,
      layoutVector.cend(),
      std::inserter(discarded, discarded.end()),
      [&](auto const& a) { return a.first; }
    );
    groups.erase(
      std::remove_if(
        groups.begin(),
        groups.end(),
        [&](auto const& a) {
          return a.layout == nullptr ||
                 discarded.find(a.layout) != discarded.end();
        }
      ), groups.end()
    );
  }

  // Apply coloring to strings, and then have each layout create its hash map.
  applyColoring(*coloring);
  std::for_each(
    layoutVector.cbegin(), coloringEnd,
    [&](auto const& layout) {
      layout.first->createColoringHashMap();
    }
  );

  // Assign sources to layouts.
  StructAnalysisResult result;
  for (auto const& group : groups) {
    assertx(group.layout);
    for (auto const source : group.profiles) {
      // TODO(mcolavita): exclude sources likely to escalate with final layout
      result.sources.insert({source, group.layout});
    }
  }

  // Assign sinks to layouts.
  for (auto const& sink : sa.merge_sinks) {
    // TODO(mcolavita): exclude sinks likely to escalate with final layout
    assertx(!sink->data->sources.empty());
    auto const it = result.sources.find(sink->data->sources.begin()->first);
    if (it == result.sources.end()) continue;
    auto const p_sampled = probabilityOfSampled(*sink);
    result.sinks.insert({sink, {ArrayLayout(it->second), p_sampled}});
  }
  return result;
}

//////////////////////////////////////////////////////////////////////////////
// Generic dispatch

ArrayLayout selectSourceLayout(
    LoggingProfile& profile, const StructAnalysisResult& sar) {
  assertx(profile.data);
  auto const mode = RO::EvalBespokeArraySpecializationMode;
  if (mode == 1 || mode == 2) return ArrayLayout::Vanilla();

  // 1. Use a struct layout if the union-find algorithm chose one.

  auto const it = sar.sources.find(&profile);
  if (it != sar.sources.end()) {
    auto const vad = profile.data->staticSampledArray;
    if (vad != nullptr) {
      auto const sad = StructDict::MakeFromVanilla(vad, it->second);
      // If the selected layout is incompatible with the static array, produce
      // vanilla instead.
      if (sad == nullptr) return ArrayLayout::Vanilla();
      profile.setStaticBespokeArray(sad);
    }
    return ArrayLayout(it->second);
  }

  // 2. If we aren't emitting monotypes, use a vanilla layout.

  if (!RO::EvalEmitBespokeMonotypes) return ArrayLayout::Vanilla();

  // 3. If the array is a runtime source, use a vanilla layout.
  // TODO(mcolavita): We can eventually support more general runtime sources.

  if (profile.key.isRuntimeLocation()) return ArrayLayout::Vanilla();

  // 4. If we escalate too often, use a vanilla layout.

  auto const p_cutoff = RO::EvalBespokeArraySourceSpecializationThreshold / 100;
  auto const p_escalated = probabilityOfEscalation(profile);
  if (p_escalated > 1 - p_cutoff) return ArrayLayout::Vanilla();

  // 5. If the array is likely to stay monotyped, use a monotype layout.

  uint64_t monotype = 0;
  uint64_t total = 0;
  for (auto const& it : profile.data->entryTypes) {
    if (EntryTypes(it.first.second).isMonotypeState()) {
      monotype += it.second;
    }
    total += it.second;
  }
  auto const p_monotype = 1.0 * monotype / total;

  if (p_monotype >= p_cutoff) {
    auto const vad = profile.data->staticSampledArray;
    if (vad == nullptr) return ArrayLayout::Vanilla();
    auto const mad = maybeMonoify(vad);
    if (mad == nullptr) return ArrayLayout::Vanilla();
    profile.setStaticBespokeArray(mad);
    return ArrayLayout(mad->layoutIndex());
  }

  return ArrayLayout::Vanilla();
}

Decision<ArrayLayout> makeSinkDecision(
    const SinkProfile& profile, const StructAnalysisResult& sar) {
  assertx(profile.data);
  using DAL = Decision<ArrayLayout>;
  auto const mode = RO::EvalBespokeArraySpecializationMode;
  if (mode == 1) return {ArrayLayout::Vanilla(), 1.0};
  if (mode == 2) return {ArrayLayout::Top(), 1.0};

  auto const it = sar.sinks.find(&profile);
  if (it != sar.sinks.end()) return it->second;

  auto const sampled = load(profile.data->sampledCount);
  auto const unsampled = load(profile.data->unsampledCount);
  if (!sampled) {
    return unsampled ? DAL{ArrayLayout::Vanilla(), 1.0}
                     : DAL{ArrayLayout::Top(), 1.0};
  }

  uint64_t vanilla = 0;
  uint64_t monotype = 0;
  uint64_t is_struct = 0;
  uint64_t total = 0;

  std::unordered_map<const bespoke::Layout*, uint64_t> structs;

  for (auto const& it : profile.data->sources) {
    auto const layout = it.first->getLayout();
    auto const count = it.second;
    if (layout.vanilla()) {
      vanilla += count;
    } else if (layout.monotype()) {
      monotype += count;
    } else if (layout.is_struct()) {
      is_struct += count;
      structs[layout.bespokeLayout()] += count;
    }
    total += count;
  }

  auto const p_cutoff = RO::EvalBespokeArraySinkSpecializationThreshold / 100;
  auto const p_sampled = 1.0 * sampled / (sampled + unsampled);

  if (!total) {
    auto const p_vanilla = 1 - p_sampled;
    if (p_vanilla >= p_cutoff) return {ArrayLayout::Vanilla(), p_vanilla};
    return {ArrayLayout::Top(), 1.0};
  }

  auto const p_vanilla = p_sampled * vanilla / total + (1 - p_sampled);
  auto const p_monotype = p_sampled * monotype / total;
  auto const p_is_struct = p_sampled * is_struct / total;

  if (p_vanilla >= p_cutoff) return {ArrayLayout::Vanilla(), p_vanilla};

  if (p_monotype >= p_cutoff) {
    using AK = ArrayData::ArrayKind;
    auto const vec = load(profile.data->arrCounts[AK::kVecKind / 2]);
    auto const dict = load(profile.data->arrCounts[AK::kDictKind / 2]);
    auto const keyset = load(profile.data->arrCounts[AK::kKeysetKind / 2]);

    assertx(vec || dict || keyset);
    if (bool(vec) + bool(dict) + bool(keyset) != 1) {
      return {ArrayLayout::Top(), 1.0};
    }

    auto const p_cutoff_val = 1 + p_cutoff - p_monotype;
    auto const dt_decision = selectValType(profile, p_cutoff_val);
    auto const dt = dt_decision.value;
    if (dt == kInvalidDataType) return {ArrayLayout::Top(), 1.0};
    auto const p_monotype_val = p_monotype + dt_decision.coverage - 1;

    if (vec) {
      return dt == KindOfUninit
        ? DAL{ArrayLayout(TopMonotypeVecLayout::Index()), p_monotype_val}
        : DAL{ArrayLayout(MonotypeVecLayout::Index(dt)), p_monotype_val};
    }

    if (dict) {
      auto const p_cutoff_key = 1 + p_cutoff - p_monotype_val;
      auto const kt_decision = selectKeyType(profile, p_cutoff_key);
      auto const kt = kt_decision.value;
      if (kt == KeyTypes::Any) return {ArrayLayout::Bespoke(), p_monotype_val};
      auto const p_monotype_key = p_monotype_val + kt_decision.coverage - 1;
      return dt == KindOfUninit
        ? DAL{ArrayLayout(TopMonotypeDictLayout::Index(kt)), p_monotype_key}
        : DAL{ArrayLayout(MonotypeDictLayout::Index(kt, dt)), p_monotype_key};
    }
  }

  if (p_is_struct >= p_cutoff) {
    for (auto const& pair : structs) {
      auto const p_this = p_sampled * pair.second / total;
      if (p_this >= p_cutoff) return {ArrayLayout(pair.first), p_this};
    }
    return {ArrayLayout(TopStructLayout::Index()), p_is_struct};
  }

  return {ArrayLayout::Top(), 1.0};
}

SinkLayout selectSinkLayout(
    const SinkProfile& profile, const StructAnalysisResult& sar) {
  auto const decision = makeSinkDecision(profile, sar);
  auto const sideExit = [&]{
    auto const p_cutoff = RO::EvalBespokeArraySinkSideExitThreshold / 100;
    if (decision.coverage < p_cutoff) return false;

    auto const count = profile.data->sources.size();
    if (count > RO::EvalBespokeArraySinkSideExitMaxSources) return false;

    auto const min_samples = RO::EvalBespokeArraySinkSideExitMinSampleCount;
    for (auto const& it : profile.data->sources) {
      if (it.second < min_samples) return false;
    }
    return true;
  }();
  return {decision.value, sideExit};
}

//////////////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////////////

ArrayLayout layoutForSource(SrcKey sk) {
  auto const profile = getLoggingProfile(sk);
  return profile ? profile->getLayout() : ArrayLayout::Vanilla();
}

SinkLayout layoutForSink(const jit::TransIDSet& ids, SrcKey sk) {
  auto result = SinkLayout{};
  for (auto const id : ids) {
    auto const profile = getSinkProfile(id, sk);
    if (!profile) continue;
    auto const sl = profile->getLayout();
    result.layout = result.layout | sl.layout;
    result.sideExit &= sl.sideExit;
  }
  if (result.layout == ArrayLayout::Bottom()) {
    return {ArrayLayout::Top(), false};
  }
  return result;
}

void selectBespokeLayouts() {
  // On successfully deserializing layout decisions, a jumpstart consumer
  // will take care of finalizing the layout hierarchy.
  if (Layout::HierarchyFinalized()) return;

  setLoggingEnabled(false);
  auto const sar = []{
    if (!RO::EvalEmitBespokeStructDicts) return StructAnalysisResult();
    StructAnalysis sa;
    eachSource([&](auto const& x) { initStructAnalysis(x, sa); });
    eachSink([&](auto const& x) { updateStructAnalysis(x, sa); });
    return finishStructAnalysis(sa);
  }();
  eachSource([&](auto& x) { x.setLayout(selectSourceLayout(x, sar)); });
  eachSink([&](auto& x) { x.setLayout(selectSinkLayout(x, sar)); });
  Layout::FinalizeHierarchy();
  startExportProfiles();
}

//////////////////////////////////////////////////////////////////////////////

}}
