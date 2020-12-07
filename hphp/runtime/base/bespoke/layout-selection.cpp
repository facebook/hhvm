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
#include "hphp/runtime/base/runtime-option.h"

namespace HPHP { namespace bespoke {

TRACE_SET_MOD(bespoke);

//////////////////////////////////////////////////////////////////////////////

namespace {

//////////////////////////////////////////////////////////////////////////////

using jit::ArrayLayout;

// Returns KeyTypes::Any if there's no good key type to specialize on.
KeyTypes selectKeyType(const SinkProfile& profile, double p_cutoff) {
  assertx(profile.data);

  auto const load = [](auto& x) { return x.load(std::memory_order_relaxed); };

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

  auto const load = [](auto& x) { return x.load(std::memory_order_relaxed); };

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

ArrayLayout selectSourceLayout(LoggingProfile& profile) {
  assertx(profile.data);

  auto const mode = RO::EvalBespokeArraySpecializationMode;
  if (mode == 1 || mode == 2) return ArrayLayout::Vanilla();

  auto const load = [](auto& x) { return x.load(std::memory_order_relaxed); };

  auto const logging = load(profile.data->loggingArraysEmitted);

  if (!logging) return ArrayLayout::Vanilla();

  uint64_t escalated = 0;
  for (auto const& it : profile.data->events) {
    auto const op = getArrayOp(it.first);
    if (op == ArrayOp::EscalateToVanilla) escalated += it.second;
  }

  auto const p_cutoff = RO::EvalBespokeArraySourceSpecializationThreshold / 100;
  auto const p_escalated = 1.0 * std::min(escalated, logging) / logging;

  if (p_escalated > 1 - p_cutoff) return ArrayLayout::Vanilla();

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
    auto const sad = profile.data->staticSampledArray;
    if (sad == nullptr) return ArrayLayout::Vanilla();
    auto const mad = maybeMonoify(sad);
    if (mad == nullptr) return ArrayLayout::Vanilla();
    profile.setStaticBespokeArray(mad);
    return ArrayLayout(mad->layoutIndex());
  }

  return ArrayLayout::Vanilla();
}

ArrayLayout selectSinkLayout(const SinkProfile& profile) {
  assertx(profile.data);

  auto const mode = RO::EvalBespokeArraySpecializationMode;
  if (mode == 1) return ArrayLayout::Vanilla();
  if (mode == 2) return ArrayLayout::Top();

  auto const load = [](auto& x) { return x.load(std::memory_order_relaxed); };

  auto const sampled = load(profile.data->sampledCount);
  auto const unsampled = load(profile.data->unsampledCount);

  if (!sampled) return unsampled ? ArrayLayout::Vanilla() : ArrayLayout::Top();

  uint64_t vanilla = 0;
  uint64_t monotype = 0;
  uint64_t total = 0;

  for (auto const& it : profile.data->sources) {
    if (it.first->layout.vanilla()) {
      vanilla += it.second;
    } else if (it.first->layout.monotype()) {
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
    auto const vec = load(profile.data->arrCounts[AK::kVecKind / 2]) +
                     load(profile.data->arrCounts[AK::kPackedKind / 2]);
    auto const dict = load(profile.data->arrCounts[AK::kDictKind / 2]) +
                      load(profile.data->arrCounts[AK::kMixedKind / 2]);
    auto const keyset = load(profile.data->arrCounts[AK::kKeysetKind / 2]);

    assertx(vec || dict || keyset);
    if (bool(vec) + bool(dict) + bool(keyset) != 1) return ArrayLayout::Top();

    auto const dt = selectValType(profile, p_cutoff);
    if (dt == kInvalidDataType) return ArrayLayout::Top();

    if (vec) {
      return dt == KindOfUninit
        ? ArrayLayout(TopMonotypeVecLayout::Index())
        : ArrayLayout(EmptyOrMonotypeVecLayout::Index(dt));
    }

    if (dict) {
      auto const kt = selectKeyType(profile, p_cutoff);
      if (kt == KeyTypes::Any) return ArrayLayout::Bespoke();
      return dt == KindOfUninit
        ? ArrayLayout(TopMonotypeDictLayout::Index(kt))
        : ArrayLayout(EmptyOrMonotypeDictLayout::Index(kt, dt));
    }
  }

  return ArrayLayout::Top();
}

//////////////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////////////

ArrayLayout layoutForSource(SrcKey sk) {
  auto const profile = getLoggingProfile(sk);
  return profile ? profile->layout : ArrayLayout::Vanilla();
}

ArrayLayout layoutForSink(const jit::TransIDSet& ids, SrcKey sk) {
  // TODO(kshaunak): Delete this block when we can ser/de bespoke profiles.
  auto const mode = RO::EvalBespokeArraySpecializationMode;
  if (mode == 1) return ArrayLayout::Vanilla();
  if (mode == 2) return ArrayLayout::Top();

  if (ids.empty()) return ArrayLayout::Top();
  auto result = ArrayLayout::Bottom();
  for (auto const id : ids) {
    auto const profile = getSinkProfile(id, sk);
    if (profile) result = result | profile->layout;
  }
  return result == ArrayLayout::Bottom() ? ArrayLayout::Top() : result;
}

void selectBespokeLayouts() {
  setLoggingEnabled(false);
  eachSource([](auto& x) { x.layout = selectSourceLayout(x); });
  eachSink([](auto& x) { x.layout = selectSinkLayout(x); });
  Layout::FinalizeHierarchy();
  startExportProfiles();
}

//////////////////////////////////////////////////////////////////////////////

}}
