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

#include "hphp/runtime/base/apc-bespoke.h"

#include "hphp/runtime/base/apc-typed-value.h"
#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/bespoke-array.h"
#include "hphp/runtime/base/bespoke/logging-array.h"
#include "hphp/runtime/base/bespoke/logging-profile.h"
#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/ext/apc/ext_apc.h"

#include <vector>

namespace HPHP {

//////////////////////////////////////////////////////////////////////////////

namespace {

TRACE_SET_MOD(bespoke);

using namespace bespoke;
using jit::ArrayLayout;

static constexpr size_t kVecHashSeed    = 0xFEC;
static constexpr size_t kDictHashSeed   = 0xD1C7;
static constexpr size_t kKeysetHashSeed = 0x5E7;

enum class APCBespokeMode { Vanilla, Logging, Bespoke };

struct APCBespokeEnv {
  const APCBespokeMode mode = APCBespokeMode::Vanilla;
  std::vector<LoggingProfile::LoggingProfileData*> logging = {};
};

const char* show(APCBespokeMode mode) {
  switch (mode) {
    case APCBespokeMode::Vanilla: return "vanilla";
    case APCBespokeMode::Logging: return "logging";
    case APCBespokeMode::Bespoke: return "bespoke";
  }
  always_assert(false);
}

template <typename Array>
ArrayData* GetEmptyArray(bool legacy) {
  if constexpr (std::is_same<Array, PackedArray>::value) {
    return ArrayData::CreateVec(legacy);
  }
  if constexpr (std::is_same<Array, MixedArray>::value) {
    return ArrayData::CreateDict(legacy);
  }
  if constexpr (std::is_same<Array, SetArray>::value) {
    return ArrayData::CreateKeyset();
  }
}

template <typename Array>
tv_lval LvalAtIterPos(ArrayData* ad, ssize_t pos) {
  if constexpr (std::is_same<Array, PackedArray>::value) {
    return PackedArray::LvalUncheckedInt(ad, pos);
  }
  if constexpr (std::is_same<Array, MixedArray>::value) {
    return &MixedArray::asMixed(ad)->data()[pos].data;
  }
  if constexpr (std::is_same<Array, SetArray>::value) {
    return &SetArray::asSet(ad)->data()[pos].tv;
  }
}

ArrayData* makeAPCBespoke(APCBespokeEnv& env, ArrayData* ad, bool hasApcTv);

/*
 * Given a static or uncounted `ain`, convert it to the layout for `env.mode`.
 *
 * Returns nullptr if we do not need to modify the input. If this function
 * returns a non-null result, then it produces an uncounted refcount on it.
 *
 * `vin` must be a vanilla array, type Array, with the same elements as `ain`.
 * `vin` may be refcounted. If so, it must have refcount 1, and this function
 * will take care of releasing it.
 */
template <typename Array, size_t Seed>
ArrayData* implAPCBespoke(APCBespokeEnv& env, ArrayData* ain,
                          ArrayData* vin, bool hasApcTv) {
  assertx(ain->isStatic() || ain->isUncounted());
  assertx(vin->isStatic() || vin->isUncounted() || vin->hasExactlyOneRef());
  assertx(vin->toDataType() == ain->toDataType());
  assertx(vin->isVanilla());
  FTRACE(2, "  Converting {}-element {} {} {} to {}:\n",
         ain->size(), ArrayLayout::FromArray(ain).describe(),
         getDataTypeString(ain->toDataType()), ain, show(env.mode));

  // We will allocate `copy` iff we need to modify the array. If we allocate
  // `copy`, then it will always have a refcount equal to 1.
  auto hash = Seed;
  ArrayData* copy = nullptr;
  std::vector<ArrayData*> subarrays;

  SCOPE_EXIT {
    FTRACE(2, "  Done with {}. copy: {}\n", ain, copy);
    assertx(IMPLIES(!copy, subarrays.empty()));
    if (!copy) return;
    for (auto const sub : subarrays) {
      DEBUG_ONLY auto const free = sub->isUncounted() && sub->uncountedDecRef();
      assertx(!free);
    }
    assertx(copy->hasExactlyOneRef());
    Array::Release(copy);
  };

  // Use direct calls to iterate over the elements of `vin` and recursively
  // apply the layout `env.mode` to them. Create `copy` if needed.
  auto const end = Array::IterEnd(vin);
  for (auto pos = Array::IterBegin(vin); pos != end;
       pos = Array::IterAdvance(vin, pos)) {
    auto const key = Array::GetPosKey(vin, pos);
    auto const val = Array::GetPosVal(vin, pos);

    // It's important to use a commutative function here, because we construct
    // many APC arrays with the same structure but with different key orders.
    if (tvIsString(key)) hash += key.val().pstr->hash();

    if (!tvIsArrayLike(val)) continue;
    auto const old_arr = val.val().parr;
    assertx(old_arr->isStatic() ^ old_arr->isUncounted());
    auto const new_arr = makeAPCBespoke(env, old_arr, false);
    if (new_arr == nullptr) continue;

    assertx(new_arr->isStatic() ^ new_arr->isUncounted());
    copy = copy ? copy : vin->isRefCounted() ? vin : Array::Copy(vin);
    assertx(copy->hasExactlyOneRef());
    assertx(Array::IterEnd(copy) == Array::IterEnd(vin));
    LvalAtIterPos<Array>(copy, pos).val().parr = new_arr;
    if (new_arr->isUncounted()) subarrays.push_back(new_arr);
  }

  // Determine what layout we should apply to the result.
  auto const vanilla = env.mode == APCBespokeMode::Vanilla;
  auto const doLookup = !vanilla && arrayTypeCouldBeBespoke(vin->toDataType());
  auto const profile = doLookup ? getLoggingProfile(APCKey{hash}) : nullptr;
  auto const layout = [&]{
    switch (env.mode) {
      case APCBespokeMode::Vanilla: return ArrayLayout::Vanilla();
      case APCBespokeMode::Logging: {
        if (!profile) return ArrayLayout::Vanilla();
        return ArrayLayout(LoggingArray::GetLayoutIndex());
      }
      case APCBespokeMode::Bespoke: {
        if (profile) return profile->getLayout();
        auto const incoming = ArrayLayout::FromArray(ain);
        return incoming.logging() ? ArrayLayout::Vanilla() : incoming;
      }
    }
    always_assert(false);
  }();

  DEBUG_ONLY auto const logging = env.mode == APCBespokeMode::Logging;
  assertx(layout.vanilla() || layout.bespokeLayout()->isConcrete());
  assertx(IMPLIES(logging, layout.vanilla() || layout.logging()));
  assertx(IMPLIES(!logging, !layout.logging()));
  FTRACE(2, "  Target layout for {}: {}\n", ain, layout.describe());

  // Return early if the input array already matches the target layout.
  if (!copy && layout == ArrayLayout::FromArray(ain)) {
    assertx(IMPLIES(vin->isRefCounted(), vin->hasExactlyOneRef()));
    if (vin->isRefCounted()) Array::Release(vin);
    if (vanilla && ain->isUncounted()) ain->setSampledArrayInPlace();
    return nullptr;
  }

  // If `vin` is counted, move it to `copy` to consume a refcount on it.
  if (vin->isRefCounted()) {
    assertx(!ain->isVanilla());
    assertx(vin->hasExactlyOneRef());
    assertx(!copy || copy == vin);
    if (!copy) copy = vin;
  }
  assertx(copy || !vin->isRefCounted());

  // To make a vanilla result, either make `copy` uncounted or use `vin`.
  // If we are targeting a vanilla layout, tag the array as being sampled.
  auto const mue = MakeUncountedEnv { /*seen=*/nullptr };
  auto const make_or_reuse_vanilla_result = [&]() -> ArrayData* {
    if (copy) {
      if (copy->empty()) return GetEmptyArray<Array>(copy->isLegacyArray());
      return Array::MakeUncounted(copy, mue, hasApcTv);
    }
    assertx(vin->isStatic() || vin->isUncounted());
    if (ain == vin) return nullptr;
    vin->persistentIncRef();
    return vin;
  };
  if (layout.vanilla()) {
    auto const vad = make_or_reuse_vanilla_result();
    if (vanilla && vad->isUncounted()) vad->setSampledArrayInPlace();
    return vad;
  }

  // We can't use generic BespokeArray methods to make a logging array because
  // we have to a) set the profile and b) avoid instrumenting creation ops.
  if (layout.logging()) {
    auto const vad = [&]{
      if (copy) {
        if (copy->empty()) return GetEmptyArray<Array>(copy->isLegacyArray());
        return Array::MakeUncounted(copy, mue, false);
      }
      assertx(vin->isStatic() || vin->isUncounted());
      vin->persistentIncRef();
      return vin;
    }();
    auto const lad = LoggingArray::MakeUncounted(vad, profile, hasApcTv);
    env.logging.push_back(lad->profile->data.get());
    return lad;
  }

  // Try to convert a vanilla array to the bespoke target layout. If this
  // layout doesn't apply, we'll just return an (unsampled) vanilla copy.
  auto const vad = copy ? copy : vin;
  auto const bad = layout.bespokeLayout()->coerce(vad);
  if (!bad) return make_or_reuse_vanilla_result();
  if (!bad->isRefCounted()) return bad;
  auto const uad = BespokeArray::MakeUncounted(bad, mue, hasApcTv);
  assertx(bad->hasExactlyOneRef());
  BespokeArray::Release(bad);
  return uad;
}

/*
 * Given a static or uncounted `ain`, convert it to the layout for `env.mode`.
 * We apply this conversion recursively to all arrays reachable from ain:
 *
 *  - Mode::Vanilla means "convert all arrays to vanilla-layout ones"
 *  - Mode::Logging means "make arrays that could be bespoke logging-layout"
 *  - Mode::Bespoke means "use layout-selection decisions for each array"
 *
 * Returns nullptr if we do not need to modify the input. If this function
 * returns a non-null result, then it produces an uncounted refcount on it.
 */
ArrayData* makeAPCBespoke(APCBespokeEnv& env, ArrayData* ain, bool hasApcTv) {
  assertx(ain->isStatic() || ain->isUncounted());

  // Escalate bespoke inputs to vanilla. Don't log it as an escalation for the
  // input, though - doing so would pessimize all sources going into APC.
  auto const vin = [&]{
    if (ain->isVanilla()) return ain;
    auto const logging = ArrayLayout::FromArray(ain).logging();
    return logging ? LoggingArray::As(ain)->wrapped
                   : BespokeArray::ToVanilla(ain, "MakeAPCBespoke");
  }();

  if (vin->isVecType()) {
    return implAPCBespoke<PackedArray, kVecHashSeed>(env, ain, vin, hasApcTv);
  } else if (vin->isDictType()) {
    return implAPCBespoke<MixedArray, kDictHashSeed>(env, ain, vin, hasApcTv);
  } else if (vin->isKeysetType()) {
    if (!arrayTypeCouldBeBespoke(KindOfKeyset)) return nullptr;
    return implAPCBespoke<SetArray, kKeysetHashSeed>(env, ain, vin, hasApcTv);
  }
  always_assert(false);
}

}

//////////////////////////////////////////////////////////////////////////////

namespace {

/*
 * A kind of "mega LoggingProfile" that allows us to do correlated sampling
 * for a complete, recursive uncounted array data structure. We only create
 * these profiles if a) bespokes are enabled and b) we're doing profiling,
 * so we'll never create them on jumpstart consumers.
 *
 * This structure contains an APCTypedValue with APCKind "StaticBespoke" or
 * "UncountedBespoke". The array in that profile is (recursively) composed
 * entirely of vanilla arrays. We'll return this array if we decide not to
 * log a given fetch.
 *
 * If we decide to log a fetch, we'll return `logging_result`. Either way,
 * we'll bump the counters in the array of logging profiles `logging` -
 * the total count only, if we did not log, and both the total and logged
 * counts, if we did.
 *
 * Finally, we need to return arrays of the selected layout after we've made
 * bespoke layout decisions (Layout::HierarchyFinalized()). To do so, we
 * track `array`. We construct it the first time we do a fetch after making
 * these decisions.
 */
struct APCBespokeData {
  APCTypedValue tv;
  mutable std::atomic<size_t> count = {};
  mutable std::atomic<ArrayData*> array = {};
  ArrayData* logging_result;
  size_t num_logging_arrays;
  LoggingProfile::LoggingProfileData* logging[1];
};

}

APCBespoke initAPCBespoke(ArrayData* ad) {
  if (!allowBespokeArrayLikes()) return { ad, nullptr };
  if (!apcExtension::UseUncounted) return { ad, nullptr };

  // After layout selection, use its results to make a (maybe bespoke) array.
  if (Layout::HierarchyFinalized()) {
    FTRACE(1, "Converting {} to bespoke on initAPCBespoke.\n", ad);
    auto env = APCBespokeEnv { APCBespokeMode::Bespoke };
    auto const result = makeAPCBespoke(env, ad, true);
    if (result == nullptr) return { ad, nullptr };
    DecRefUncountedArray(ad);
    return { result, nullptr };
  }

  // The array may recursively contain bespoke arrays. Make it deeply vanilla.
  ad = [&]{
    FTRACE(1, "Converting {} to vanilla on initAPCBespoke.\n", ad);
    auto env = APCBespokeEnv { APCBespokeMode::Vanilla };
    auto const result = makeAPCBespoke(env, ad, false);
    if (result == nullptr) return ad;
    DecRefUncountedArray(ad);
    return result;
  }();
  if (!RO::EvalEmitAPCBespokeArrays) return { ad, nullptr };

  // Check if we need to do logging for this array. We'll need to do so if it
  // supports bespoke layouts, *or if any of its recursive subarrays does*.
  FTRACE(1, "Converting {} to logging on initAPCBespoke.\n", ad);
  auto env = APCBespokeEnv { APCBespokeMode::Logging };
  auto const result = makeAPCBespoke(env, ad, false);
  if (result == nullptr) return { ad, nullptr };

  // We need to do logging. Allocate an APCBespokeData.
  assertx(!env.logging.empty());
  auto const count = env.logging.size();
  auto const bytes = sizeof(APCBespokeData) + sizeof(env.logging[0]) * count;
  auto const data = reinterpret_cast<APCBespokeData*>(malloc(bytes));

  // Fill in fields of the APCBespokeData and return it.
  data->count.store(0, std::memory_order_release);
  data->array.store(nullptr, std::memory_order_release);
  data->logging_result = result;
  data->num_logging_arrays = env.logging.size();
  for (auto i = 0; i < env.logging.size(); i++) {
    data->logging[i] = env.logging[i];
  }
  return { ad, &data->tv };
}

ArrayData* readAPCBespoke(const APCTypedValue* tv) {
  static_assert(offsetof(APCBespokeData, tv) == 0);

  auto const data = reinterpret_cast<const APCBespokeData*>(tv);
  if (auto const ad = data->array.load(std::memory_order_acquire)) return ad;
  auto const vad = tv->getArrayData();

  // After layout selection, remake the array as a (maybe bespoke) array.
  // TODO(kshaunak): We can treadmill the vanilla and logging arrays now.
  if (Layout::HierarchyFinalized()) {
    FTRACE(1, "Converting {} to bespoke on readAPCBespoke.\n", vad);
    auto const result = [&]{
      auto env = APCBespokeEnv { APCBespokeMode::Bespoke };
      auto const attempt = makeAPCBespoke(env, vad, false);
      if (attempt != nullptr) return attempt;
      vad->persistentIncRef();
      return vad;
    }();
    ArrayData* expected = nullptr;
    if (!data->array.compare_exchange_strong(expected, result)) {
      DecRefUncountedArray(result);
      return expected;
    }
    // Clean up after ourselves: treadmill the vanilla and logging arrays.
    auto const lad = data->logging_result;
    const_cast<APCTypedValue*>(tv)->setArrayData(result);
    Treadmill::enqueue([vad, lad]{
      DecRefUncountedArray(vad);
      DecRefUncountedArray(lad);
    });
    return result;
  }

  // Before layout selection, use sample counts to choose whether to log.
  // TODO(kshaunak): Should we also gate on g_emitLoggingArrays here?
  auto const count = data->count++;
  if (count % RO::EvalEmitLoggingArraySampleRate != 1) {
    for (auto i = 0; i < data->num_logging_arrays; i++) {
      data->logging[i]->sampleCount++;
    }
    return vad;
  }

  for (auto i = 0; i < data->num_logging_arrays; i++) {
    data->logging[i]->sampleCount++;
    data->logging[i]->loggingArraysEmitted++;
  }
  return data->logging_result;
}

void freeAPCBespoke(APCTypedValue* tv) {
  auto const data = reinterpret_cast<APCBespokeData*>(tv);
  if (auto const ad = data->array.load(std::memory_order_acquire)) {
    DecRefUncountedArray(ad);
  } else {
    DecRefUncountedArray(tv->getArrayData());
    DecRefUncountedArray(data->logging_result);
  }
  free(data);
}

//////////////////////////////////////////////////////////////////////////////

}
