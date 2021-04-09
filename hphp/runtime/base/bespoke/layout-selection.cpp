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

using jit::ArrayLayout;

uint64_t load(const std::atomic<uint64_t>& x) {
  return x.load(std::memory_order_relaxed);
}

double probabilityOfEscalation(const LoggingProfile& profile) {
  auto const logging = load(profile.data->loggingArraysEmitted);
  if (!logging) return 1.0;

  uint64_t escalated = 0;
  for (auto const& it : profile.data->events) {
    auto const op = getArrayOp(it.first);
    if (op == ArrayOp::EscalateToVanilla) escalated += it.second;
  }
  return 1.0 * std::min(escalated, logging) / logging;
}

//////////////////////////////////////////////////////////////////////////////
// Monotype helpers

// Returns KeyTypes::Any if there's no good key type to specialize on.
KeyTypes selectKeyType(const SinkProfile& profile, double p_cutoff) {
  assertx(profile.data);

  auto const empty = load(profile.data->keyCounts[int(KeyTypes::Empty)]);
  auto const ints  = load(profile.data->keyCounts[int(KeyTypes::Ints)]);
  auto const strs  = load(profile.data->keyCounts[int(KeyTypes::Strings)]);
  auto const sstrs = load(profile.data->keyCounts[int(KeyTypes::StaticStrings)]);
  auto const any   = load(profile.data->keyCounts[int(KeyTypes::Any)]);

  auto const total = empty + ints + strs + sstrs + any;
  if (!total) return KeyTypes::Any;

  auto const p_ints  = 1.0 * (empty + ints) / total;
  auto const p_strs  = 1.0 * (empty + strs + sstrs) / total;
  auto const p_sstrs = 1.0 * (empty + sstrs) / total;

  if (p_sstrs >= p_cutoff) return KeyTypes::StaticStrings;
  if (p_strs >= p_cutoff)  return KeyTypes::Strings;
  if (p_ints >= p_cutoff)  return KeyTypes::Ints;
  return KeyTypes::Any;
}

// Returns kKindOfUninit if we should specialize on "monotype of unknown type".
// Returns kInvalidDataType if we should not specialize this sink on a monotype.
DataType selectValType(const SinkProfile& profile, double p_cutoff) {
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

  if (!total) return kInvalidDataType;

  auto const p_empty = 1.0 * empty / total;
  auto const p_max   = 1.0 * (empty + max_count) / total;
  auto const p_mono  = 1.0 * (total - any) / total;

  // TODO(kshaunak): We may want to specialize on empty in the future.
  if (p_empty >= p_cutoff) return KindOfUninit;
  if (p_max >= p_cutoff)   return safe_cast<DataType>(max_index + kMinDataType);
  if (p_mono >= p_cutoff)  return KindOfUninit;
  return kInvalidDataType;
}

//////////////////////////////////////////////////////////////////////////////
// Struct helpers

struct StructAnalysis {
  std::vector<KeyOrder> key_orders;
  std::vector<const SinkProfile*> merge_sinks;
  UnionFind<const LoggingProfile*> union_find;
};

struct StructAnalysisResult {
  folly::F14FastMap<const LoggingProfile*, const StructLayout*> sources;
  folly::F14FastMap<const SinkProfile*, const StructLayout*> sinks;
};

// Returns true if we treat the given sink as a "merge point" and union the
// sets of sinks that are incident to that merge. These points are also the
// only ones at which we'll JIT struct access code.
bool mergeStructsAtSink(const SinkProfile& profile, const StructAnalysis& sa) {
  // 1. We must have sufficient LoggingArray coverage for the given sink.
  if (profile.data->sources.empty()) return false;
  auto const sampled = load(profile.data->sampledCount);
  auto const unsampled = load(profile.data->unsampledCount);
  auto const p_cutoff = RO::EvalBespokeArraySinkSpecializationThreshold / 100;
  auto const p_sampled = 1.0 * sampled / (sampled + unsampled);
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
    auto const vad = profile.data->staticSampledArray;
    if (vad != nullptr && vad->isDictType()) return true;
    if (profile.key.isRuntimeLocation()) return true;
    auto const op = profile.key.op();
    return op == OpNewDictArray || op == OpNewStructDict;
  }();

  // Add a node for this source to our union-find table if the type matches
  // and there's a struct layout that's a good match for this source alone.
  if (!type_okay || profile.data->keyOrders.empty()) return;
  auto const ko = collectKeyOrder(profile.data->keyOrders);
  if (!ko.valid()) return;
  DEBUG_ONLY auto const index = sa.union_find.insert(&profile);
  assertx(index == sa.key_orders.size());
  sa.key_orders.push_back(ko);
}

void updateStructAnalysis(const SinkProfile& profile, StructAnalysis& sa) {
  auto const& sources = profile.data->sources;
  if (sources.empty() || !mergeStructsAtSink(profile, sa)) return;
  auto const root = profile.data->sources.begin()->first;
  for (auto const& pair : sources) {
    sa.union_find.merge(root, pair.first);
  }
  sa.merge_sinks.push_back(&profile);
}

StructAnalysisResult finishStructAnalysis(StructAnalysis& sa) {
  auto const p_cutoff = RO::EvalBespokeArraySourceSpecializationThreshold / 100;
  std::vector<std::pair<std::vector<const LoggingProfile*>, double>> groups;

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
      groups.push_back({std::move(group), weight});
    }
  });
  std::sort(groups.begin(), groups.end(),
            [](auto const& a, auto const& b) { return a.second > b.second; });

  // Assign sources to layouts.
  StructAnalysisResult result;
  for (auto const& group : groups) {
    KeyOrderMap kom;
    for (auto const source : group.first) {
      auto const index = sa.union_find.lookup(source);
      kom.emplace(sa.key_orders[*index], 0).first->second.value++;
    }
    auto const ko = collectKeyOrder(kom);
    auto const layout = StructLayout::GetLayout(ko, true);
    if (layout != nullptr) {
      for (auto const source : group.first) {
        result.sources.insert({source, layout});
      }
    }
  }

  // Assign sinks to layouts.
  for (auto const& sink : sa.merge_sinks) {
    assertx(!sink->data->sources.empty());
    auto const it = result.sources.find(sink->data->sources.begin()->first);
    if (it != result.sources.end()) result.sinks.insert({sink, it->second});
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
      profile.setStaticBespokeArray(sad);
    }
    return ArrayLayout(it->second);
  }

  // 2. If we aren't emitting monotypes, use a vanilla layout.

  if (!RO::EvalEmitBespokeMonotypes) return ArrayLayout::Vanilla();

  // 3. If the array is a runtime source, use a vanilla layout.
  // TODO(mcolavita): We can eventually support more general runtime sources.

  if (profile.key.isRuntimeLocation()) {
    return ArrayLayout::Vanilla();
  }

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

ArrayLayout selectSinkLayout(
    const SinkProfile& profile, const StructAnalysisResult& sar) {
  assertx(profile.data);
  auto const mode = RO::EvalBespokeArraySpecializationMode;
  if (mode == 1) return ArrayLayout::Vanilla();
  if (mode == 2) return ArrayLayout::Top();

  auto const it = sar.sinks.find(&profile);
  if (it != sar.sinks.end()) return ArrayLayout(it->second);

  auto const sampled = load(profile.data->sampledCount);
  auto const unsampled = load(profile.data->unsampledCount);
  if (!sampled) return unsampled ? ArrayLayout::Vanilla() : ArrayLayout::Top();

  uint64_t vanilla = 0;
  uint64_t monotype = 0;
  uint64_t total = 0;

  for (auto const& it : profile.data->sources) {
    auto const layout = it.first->getLayout();
    if (layout.vanilla()) {
      vanilla += it.second;
    } else if (layout.monotype()) {
      monotype += it.second;
    }
    total += it.second;
  }

  auto const p_cutoff = RO::EvalBespokeArraySinkSpecializationThreshold / 100;
  auto const p_sampled = 1.0 * sampled / (sampled + unsampled);

  if (!total) {
    if ((1 - p_sampled) >= p_cutoff) return ArrayLayout::Vanilla();
    return ArrayLayout::Top();
  }

  auto const p_vanilla = p_sampled * vanilla / total + (1 - p_sampled);
  auto const p_monotype = p_sampled * monotype / total;

  if (p_vanilla >= p_cutoff) return ArrayLayout::Vanilla();

  if (p_monotype >= p_cutoff) {
    using AK = ArrayData::ArrayKind;
    auto const vec = load(profile.data->arrCounts[AK::kVecKind / 2]);
    auto const dict = load(profile.data->arrCounts[AK::kDictKind / 2]);
    auto const keyset = load(profile.data->arrCounts[AK::kKeysetKind / 2]);

    assertx(vec || dict || keyset);
    if (bool(vec) + bool(dict) + bool(keyset) != 1) return ArrayLayout::Top();

    auto const dt = selectValType(profile, p_cutoff);
    if (dt == kInvalidDataType) return ArrayLayout::Top();

    if (vec) {
      return dt == KindOfUninit
        ? ArrayLayout(TopMonotypeVecLayout::Index())
        : ArrayLayout(MonotypeVecLayout::Index(dt));
    }

    if (dict) {
      auto const kt = selectKeyType(profile, p_cutoff);
      if (kt == KeyTypes::Any) return ArrayLayout::Bespoke();
      return dt == KindOfUninit
        ? ArrayLayout(TopMonotypeDictLayout::Index(kt))
        : ArrayLayout(MonotypeDictLayout::Index(kt, dt));
    }
  }

  return ArrayLayout::Top();
}

//////////////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////////////

ArrayLayout layoutForSource(SrcKey sk) {
  auto const profile = getLoggingProfile(sk);
  return profile ? profile->getLayout() : ArrayLayout::Vanilla();
}

ArrayLayout layoutForSink(const jit::TransIDSet& ids, SrcKey sk) {
  if (ids.empty()) return ArrayLayout::Top();
  auto result = ArrayLayout::Bottom();
  for (auto const id : ids) {
    auto const profile = getSinkProfile(id, sk);
    if (profile) result = result | profile->getLayout();
  }
  return result == ArrayLayout::Bottom() ? ArrayLayout::Top() : result;
}

void selectBespokeLayouts() {
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
