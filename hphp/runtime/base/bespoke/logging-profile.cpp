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

#include "hphp/runtime/base/bespoke/logging-profile.h"

#include "hphp/runtime/base/bespoke/logging-array.h"
#include "hphp/runtime/base/bespoke/layout.h"
#include "hphp/runtime/base/bespoke/struct-dict.h"
#include "hphp/runtime/base/memory-manager-defs.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/server/memory-stats.h"
#include "hphp/runtime/vm/jit/mcgen-translate.h"
#include "hphp/runtime/vm/vm-regs.h"

#include <folly/SharedMutex.h>
#include <folly/String.h>
#include <tbb/concurrent_hash_map.h>

#include <algorithm>
#include <atomic>
#include <sstream>

namespace HPHP { namespace bespoke {

TRACE_SET_MOD(bespoke);

//////////////////////////////////////////////////////////////////////////////

namespace {

template <typename T, typename U>
bool fits(U u) {
  return u >= std::numeric_limits<T>::min() &&
         u <= std::numeric_limits<T>::max();
}
constexpr int8_t kInt8Min = std::numeric_limits<int8_t>::min();

// The number of events we render inline before dropping key specializations.
constexpr size_t kMaxNumDetailedEvents = 32;

folly::SharedMutex s_profilingLock;
std::atomic<bool> s_profiling{true};

std::string arrayOpToString(ArrayOp op) {
  switch (op) {
#define X(name, read) case ArrayOp::name: return #name;
ARRAY_OPS
#undef X
  }
  not_reached();
}

bool arrayOpIsRead(ArrayOp op) {
  switch (op) {
#define X(name, read) case ArrayOp::name: return read;
ARRAY_OPS
#undef X
  }
  not_reached();
}

// SrcKey includes more information than just the (Func, Offset) pair,
// but we want all our logging to be grouped by these two fields alone.
SrcKey canonicalize(SrcKey sk) {
  assertx(sk.valid());
  return SrcKey(sk.func(), sk.offset(), ResumeMode::None);
}

// If the given source always produces a particular static array, return it.
ArrayData* getStaticArray(LoggingProfileKey key) {
  switch (key.locationType) {
    case LocationType::Runtime:
      return nullptr;

    case LocationType::Property: {
      auto const index = key.cls->propSlotToIndex(key.slot);
      auto const tv = key.cls->declPropInit()[index].val.tv();
      return tvIsArrayLike(tv) ? tv.val().parr : nullptr;
    }

    case LocationType::SrcKey: {
      auto const op = key.sk.op();
      if (op != Op::Vec && op != Op::Dict && op != Op::Keyset) {
        return nullptr;
      }
      auto const unit = key.sk.func()->unit();
      auto const result = unit->lookupArrayId(getImm(key.sk.pc(), 0).u_AA);
      return const_cast<ArrayData*>(result);
    }
  }
  always_assert(false);
}

template <typename M, typename L, typename K>
size_t incrementCounter(M& map, L& mutex, K key) {
  {
    folly::SharedMutex::ReadHolder lock{mutex};
    auto it = map.find(key);
    if (it != map.end()) {
      it->second.value++;
      return it->second;
    }
  }
  folly::SharedMutex::WriteHolder lock{mutex};
  auto [it, didInsert] = map.emplace(key, 0);
  it->second.value++;
  return it->second;
}

}

//////////////////////////////////////////////////////////////////////////////

// The key for a sampled event. The granularity we choose is important here:
// too fine, and our profiles will have too many entries; too coarse, and we
// won't be able to make certain optimizations.
struct alignas(8) EventKey {
  explicit EventKey(ArrayOp op);
  EventKey(ArrayOp op, int64_t k);
  EventKey(ArrayOp op, const StringData* k);
  EventKey(ArrayOp op, TypedValue v);
  EventKey(ArrayOp op, int64_t k, TypedValue v);
  EventKey(ArrayOp op, const StringData* k, TypedValue v);

  explicit EventKey(uint64_t value) {
    static_assert(sizeof(EventKey) == sizeof(uint64_t), "");
    memmove(this, &value, sizeof(EventKey));
  }

  EventKey dropKeySpecialization() const {
    auto result = *this;
    result.m_key = kInvalidId;
    return result;
  }

  uint64_t toUInt64() const {
    uint64_t value;
    memmove(&value, this, sizeof(EventKey));
    return value;
  }

  ArrayOp getOp() const {
    return m_op;
  }

  std::string toString() const;

private:
  // We specialize on certain subtypes that we can represent efficiently.
  // Str32 is a static string with a 4-byte pointer, even in non-lowptr builds.
  //
  // Spec is strictly more specific than DataType, because we drop persistence
  // when saving types to the event frequency map.
  enum class Spec : uint8_t { None, Int8, Int16, Int32, Int64, Str32, Str };

  Spec getSpec(TypedValue v) {
    if (tvIsString(v)) {
      if (!val(v).pstr->isStatic()) return Spec::Str;
      auto const str = uintptr_t(val(v).pstr);
      return fits<uint32_t>(str) ? Spec::Str32 : Spec::Str;
    }
    if (tvIsInt(v)) {
      auto const num = val(v).num;
      if (fits<int8_t>(num))  return Spec::Int8;
      if (fits<int16_t>(num)) return Spec::Int16;
      if (fits<int32_t>(num)) return Spec::Int32;
      return Spec::Int64;
    }
    return Spec::None;
  }

  void setOp(ArrayOp op) {
    m_op = op;
  }
  void setKey(int64_t k) {
    m_key_spec = getSpec(make_tv<KindOfInt64>(k));
    if (m_key_spec == Spec::Int8) m_key = safe_cast<uint32_t>(k - kInt8Min);
  }
  void setKey(const StringData* k) {
    m_key_spec = getSpec(make_tv<KindOfString>(const_cast<StringData*>(k)));
    if (m_key_spec == Spec::Str32) m_key = safe_cast<uint32_t>(uintptr_t(k));
  }
  void setVal(TypedValue v) {
    m_val_spec = getSpec(v);
    m_val_type = dt_modulo_persistence(type(v));
  }

  ArrayOp m_op;
  Spec m_key_spec = Spec::None;
  Spec m_val_spec = Spec::None;
  DataType m_val_type = kInvalidDataType;
  uint32_t m_key = kInvalidId; // May be set for Spec::Int8 and Spec::Str32
};

// Dispatch to the setters above. There must be a better way...
EventKey::EventKey(ArrayOp op) { setOp(op); }
EventKey::EventKey(ArrayOp op, int64_t k) { setOp(op); setKey(k); }
EventKey::EventKey(ArrayOp op, const StringData* k) { setOp(op); setKey(k); }
EventKey::EventKey(ArrayOp op, TypedValue v) { setOp(op); setVal(v); }
EventKey::EventKey(ArrayOp op, int64_t k, TypedValue v)
  { setOp(op); setKey(k); setVal(v); }
EventKey::EventKey(ArrayOp op, const StringData* k, TypedValue v)
  { setOp(op); setKey(k); setVal(v); }

std::string EventKey::toString() const {
  auto const op = arrayOpToString(m_op);
  auto const specToStr = [](EventKey::Spec spec) {
    switch (spec) {
      case Spec::None:  return "none";
      case Spec::Int8:  return "i8";
      case Spec::Int16: return "i16";
      case Spec::Int32: return "i32";
      case Spec::Int64: return "i64";
      case Spec::Str32: return "s32";
      case Spec::Str:   return "str";
    }
    not_reached();
  };
  auto const key = [&]() -> std::string {
    auto const spec = m_key_spec;
    if (spec == Spec::None) return "";
    if (m_key != kInvalidId) {
      assertx(spec == Spec::Int8 || spec == Spec::Str32);
      if (spec == Spec::Int8) {
        auto const i = safe_cast<int8_t>(safe_cast<int16_t>(m_key) + kInt8Min);
        return folly::sformat(" key=[i8:{}]", i);
      } else {
        auto const s = ((StringData*)safe_cast<uintptr_t>(m_key))->data();
        return folly::sformat(" key=[s32:\"{}\"]", folly::cEscape<std::string>(s));
      }
    }
    return folly::sformat(" key=[{}]", specToStr(spec));
  }();
  auto const val = [&]() -> std::string {
    if (m_val_type == kInvalidDataType) return "";
    auto const spec = m_val_spec;
    return spec == Spec::None ? folly::sformat(" val=[{}]", tname(m_val_type))
                              : folly::sformat(" val=[{}]", specToStr(spec));
  }();
  return folly::sformat("{}{}{}", op, key, val);
}

//////////////////////////////////////////////////////////////////////////////

LoggingProfile::LoggingProfile(LoggingProfileKey key)
  : key(key)
  , data(std::make_unique<LoggingProfileData>())
{
  assertx(s_profiling.load(std::memory_order_acquire));
}

LoggingProfile::LoggingProfile(LoggingProfileKey key, jit::ArrayLayout layout)
  : key(key)
  , layout(layout)
  , staticBespokeArray(nullptr)
{
  if (layout.vanilla()) return;
  if (auto const ad = getStaticArray(key)) {
    setStaticBespokeArray(BespokeArray::asBespoke(layout.apply(ad)));
  }
}

double LoggingProfile::getSampleCountMultiplier() const {
  assertx(data);
  if (data->loggingArraysEmitted == 0) return 0;

  return data->sampleCount / (double) data->loggingArraysEmitted;
}

uint64_t LoggingProfile::getTotalEvents() const {
  assertx(data);
  size_t total = 0;
  for (auto const& event : data->events) total += event.second;

  return total;
}

double LoggingProfile::getProfileWeight() const {
  return getTotalEvents() * getSampleCountMultiplier();
}

void LoggingProfile::logEvent(ArrayOp op) {
  logEventImpl(EventKey(op));
}
void LoggingProfile::logEvent(ArrayOp op, int64_t k) {
  logEventImpl(EventKey(op, k));
}
void LoggingProfile::logEvent(ArrayOp op, const StringData* k) {
  logEventImpl(EventKey(op, k));
}
void LoggingProfile::logEvent(ArrayOp op, TypedValue v) {
  logEventImpl(EventKey(op, v));
}
void LoggingProfile::logEvent(ArrayOp op, int64_t k, TypedValue v) {
  logEventImpl(EventKey(op, k, v));
}
void LoggingProfile::logEvent(ArrayOp op, const StringData* k, TypedValue v) {
  logEventImpl(EventKey(op, k, v));
}

void LoggingProfile::logEventImpl(const EventKey& key) {
  // Hold the read mutex for the duration of the mutation so that profiling
  // cannot be interrupted until the mutation is complete.
  folly::SharedMutex::ReadHolder lock{s_profilingLock};
  if (!s_profiling.load(std::memory_order_acquire)) return;

  assertx(data);

  DEBUG_ONLY auto const count =
    incrementCounter(data->events, data->mapLock, key.toUInt64());
  DEBUG_ONLY auto const has_sink = key.getOp() != ArrayOp::ReleaseUncounted;
  DEBUG_ONLY auto const sink = has_sink ? getSrcKey() : SrcKey{};
  FTRACE(6, "{} -> {}: {} [count={}]\n", key.toString(),
         (sink.valid() ? sink.getSymbol() : "<unknown>"),
         EventKey(key).toString(), count);
}

void LoggingProfile::logEntryTypes(EntryTypes before, EntryTypes after) {
  // Hold the read mutex for the duration of the mutation so that profiling
  // cannot be interrupted until the mutation is complete.
  folly::SharedMutex::ReadHolder lock{s_profilingLock};
  if (!s_profiling.load(std::memory_order_acquire)) return;

  assertx(data);

  auto const key = std::make_pair(before.asInt16(), after.asInt16());
  incrementCounter(data->entryTypes, data->mapLock, key);
}

void LoggingProfile::logKeyOrders(const KeyOrder& ko) {
  // Hold the read mutex for the duration of the mutation so that profiling
  // cannot be interrupted until the mutation is complete.
  folly::SharedMutex::ReadHolder lock{s_profilingLock};
  if (!s_profiling.load(std::memory_order_acquire)) return;

  assertx(data);

  incrementCounter(data->keyOrders, data->mapLock, ko);
}

BespokeArray* LoggingProfile::getStaticBespokeArray() const {
  return staticBespokeArray;
}

void LoggingProfile::setStaticBespokeArray(BespokeArray* bad) {
  assertx(bad != nullptr);
  assertx(staticBespokeArray == nullptr);
  assertx(getStaticArray(key) != nullptr);

  staticBespokeArray = bad;

  if (key.locationType == LocationType::Property) {
    auto const index = key.cls->propSlotToIndex(key.slot);
    auto props = const_cast<Class::PropInitVec*>(&key.cls->declPropInit());
    auto const lval = (*props)[index].val;
    assertx(tvIsArrayLike(lval));
    lval.val().parr = bad;
  }
}

jit::ArrayLayout LoggingProfile::getLayout() const {
  return layout.load(std::memory_order_acquire);
}

void LoggingProfile::setLayout(jit::ArrayLayout layout) {
  this->layout.store(layout, std::memory_order_release);
  if (key.isRuntimeLocation() && layout.is_struct()) {
    // TODO(mcolavita): setLayout should trigger setStaticBespokeArray.
    key.runtimeStruct->applyLayout(StructLayout::As(layout.bespokeLayout()));
  }
}

//////////////////////////////////////////////////////////////////////////////

SinkProfile::SinkProfile(SinkProfileKey key)
  : key(key)
  , data(std::make_unique<SinkProfileData>())
{
  assertx(s_profiling.load(std::memory_order_acquire));
}

SinkProfile::SinkProfile(SinkProfileKey key, jit::ArrayLayout layout)
  : key(key)
  , layout(layout)
{}

jit::ArrayLayout SinkProfile::getLayout() const {
  return layout.load(std::memory_order_acquire);
}

void SinkProfile::setLayout(jit::ArrayLayout layout) {
  this->layout.store(layout, std::memory_order_release);
}

void SinkProfile::update(const ArrayData* ad) {
  folly::SharedMutex::ReadHolder lock{s_profilingLock};
  if (!s_profiling.load(std::memory_order_acquire)) return;

  assertx(data);

  // Because profiling hasn't stopped yet, any bespoke arrays should be
  // LoggingArrays at this point. Bail out if we get a non-logging bespoke.
  const LoggingArray* lad = nullptr;
  if (!ad->isVanilla()) {
    auto const index = BespokeArray::asBespoke(ad)->layoutIndex();
    if (index != LoggingArray::GetLayoutIndex()) return;
    lad = LoggingArray::As(ad);
  }

  // Update array-like-generic fields: the sampled bit and the array type.

  if (lad != nullptr || ad->isSampledArray()) {
    data->sampledCount++;
  } else {
    data->unsampledCount++;
  }

  auto const kind = ad->kind() / 2;
  assertx(kind < kNumArrTypes);
  data->arrCounts[kind]++;

  if (lad == nullptr) return;

  // Update LoggingArray-only fields: key type, val type, and array source.
  auto const& et = lad->entryTypes;
  auto const key = size_t(et.keyTypes);
  auto const val = [&]{
    if (et.valueTypes == ValueTypes::Empty) return kNoValTypes;
    if (et.valueTypes != ValueTypes::Monotype) return kAnyValType;
    auto const dt = dt_modulo_persistence(et.valueDatatype);
    return size_t(static_cast<data_type_t>(dt) - kMinDataType);
  }();

  assertx(key < kNumKeyTypes);
  assertx(val < kNumValTypes);
  data->keyCounts[key]++;
  data->valCounts[val]++;

  incrementCounter(data->sources, data->mapLock, lad->profile);
  incrementCounter(data->keyOrders, data->mapLock, lad->keyOrder);
}

//////////////////////////////////////////////////////////////////////////////

namespace {

struct EventOutputData {
  EventOutputData(EventKey event, uint64_t count): event(event), count(count) {}

  bool operator<(const EventOutputData& other) const {
    return count > other.count;
  }

  EventKey event;
  uint64_t count;
};

struct OperationOutputData {
  explicit OperationOutputData(ArrayOp operation)
    : operation(operation)
    , totalCount(0)
  {}

  bool operator<(const OperationOutputData& other) const {
    return totalCount > other.totalCount;
  }

  void registerEvent(EventKey key, uint64_t count) {
    events.emplace_back(key, count);
    totalCount += count;
  }

  void formatEvents() {
    if (events.size() > kMaxNumDetailedEvents) {
      std::map<uint64_t, uint64_t> deduped;
      for (auto const& event : events) {
        deduped[event.event.dropKeySpecialization().toUInt64()] += event.count;
      }
      events.clear();
      for (auto const& pair : deduped) {
        events.emplace_back(EventKey(pair.first), pair.second);
      }
    }
    std::sort(events.begin(), events.end());
  }

  ArrayOp operation;
  std::vector<EventOutputData> events;
  uint64_t totalCount;
};

struct EntryTypesUseOutputData {
  EntryTypesUseOutputData(EntryTypes state, uint64_t count)
    : state(state)
    , count(count)
  {}

  bool operator<(const EntryTypesUseOutputData& other) const {
    return count > other.count;
  }

  EntryTypes state;
  uint64_t count;
};

struct KeyOrderOutputData {
  KeyOrderOutputData(const KeyOrder& ko, uint64_t count)
    : ko(ko)
    , count(count)
  {}

  bool operator<(const KeyOrderOutputData& other) const {
    return count > other.count;
  }

  KeyOrder ko;
  uint64_t count;
};

struct EntryTypesEscalationOutputData {
  EntryTypesEscalationOutputData(EntryTypes before, EntryTypes after,
                               uint64_t count)
    : before(before)
    , after(after)
    , count(count)
  {}

  bool operator<(const EntryTypesEscalationOutputData& other) const {
    return count > other.count;
  }

  EntryTypes before;
  EntryTypes after;
  uint64_t count;
};

struct SourceFrequencyData {
  SourceFrequencyData(const std::string& source, uint64_t count)
    : source(source), count(count) {}

  bool operator<(const SourceFrequencyData& other) const {
    return count > other.count;
  }

  std::string source;
  uint64_t count;
};

struct SinkTypeData {
  SinkTypeData(const char* name, uint64_t count) : name(name), count(count) {}

  bool operator<(const SinkTypeData& other) const {
    return count > other.count;
  }

  const char* name;
  uint64_t count;
};

// NOTE: These helpers undo the transformations in SinkProfile::update.

const char* getArrTypeStr(size_t type) {
  assertx(type <= SinkProfile::kNumArrTypes);
  return ArrayData::kindToString(ArrayData::ArrayKind(type * 2));
}

const char* getKeyTypeStr(size_t type) {
  assertx(type <= SinkProfile::kNumKeyTypes);
  return show(KeyTypes(type));
}

const char* getValTypeStr(size_t type) {
  assertx(type <= SinkProfile::kNumValTypes);
  if (type == SinkProfile::kNoValTypes) return "Empty";
  if (type == SinkProfile::kAnyValType) return "Any";
  auto const dt = DataType(safe_cast<int8_t>(type) + kMinDataType);
  switch (dt) {
#define DT(name, ...) case KindOf##name: return #name;
DATATYPES
#undef DT
  }
  always_assert(false);
}

template <size_t N, typename Fn>
std::vector<SinkTypeData> populateSortedCounts(
    const std::atomic<uint64_t> (&counts)[N], Fn fn) {
  std::vector<SinkTypeData> result;
  for (auto i = 0; i < N; i++) {
    auto const count = counts[i].load(std::memory_order_relaxed);
    if (count) result.push_back({fn(i), count});
  }
  std::sort(result.begin(), result.end());
  return result;
}

struct SourceOutputData {
  SourceOutputData(const LoggingProfile* profile,
                   std::vector<OperationOutputData>&& operations,
                   std::vector<EntryTypesEscalationOutputData>&& monoEscalations,
                   std::vector<EntryTypesUseOutputData>&& monoUses,
                   std::vector<KeyOrderOutputData>&& kos)
    : profile(profile)
    , monotypeEscalations(std::move(monoEscalations))
    , monotypeUses(std::move(monoUses))
    , keyOrders(std::move(kos))
  {
    std::sort(monotypeEscalations.begin(), monotypeEscalations.end());
    std::sort(monotypeUses.begin(), monotypeUses.end());
    std::sort(operations.begin(), operations.end());
    std::sort(keyOrders.begin(), keyOrders.end());

    readCount = 0;
    writeCount = 0;
    for (auto const& operationData : operations) {
      if (arrayOpIsRead(operationData.operation)) {
        readCount += operationData.totalCount;
        readOperations.push_back(operationData);
      } else {
        writeCount += operationData.totalCount;
        writeOperations.push_back(operationData);
      }
    }

    weight = profile->getProfileWeight();
  }

  bool operator<(const SourceOutputData& other) const {
    return weight > other.weight;
  }

  const LoggingProfile* profile;
  std::vector<OperationOutputData> readOperations;
  std::vector<OperationOutputData> writeOperations;
  std::vector<EntryTypesEscalationOutputData> monotypeEscalations;
  std::vector<EntryTypesUseOutputData> monotypeUses;
  std::vector<KeyOrderOutputData> keyOrders;
  uint64_t readCount;
  uint64_t writeCount;
  double weight;
};

struct SinkOutputData {
  explicit SinkOutputData(const SinkProfile* profile)
    : profile(profile)
  {
    assertx(profile->data);

    std::map<std::string, uint64_t> sourceCounts;
    for (auto const& it : profile->data->sources) {
      sourceCounts[it.first->key.toString()] += it.second;
      loggedCount += it.second;
    }
    for (auto const& it : sourceCounts) {
      sources.push_back({it.first, it.second});
    }
    std::sort(sources.begin(), sources.end());

    arrCounts = populateSortedCounts(profile->data->arrCounts, getArrTypeStr);
    keyCounts = populateSortedCounts(profile->data->keyCounts, getKeyTypeStr);
    valCounts = populateSortedCounts(profile->data->valCounts, getValTypeStr);

    sampledCount = profile->data->sampledCount.load(std::memory_order_relaxed);
    unsampledCount = profile->data->unsampledCount.load(std::memory_order_relaxed);

    weight = sampledCount + unsampledCount;

    for (auto const& keyOrderAndCount : profile->data->keyOrders) {
      keyOrders.push_back({keyOrderAndCount.first, keyOrderAndCount.second});
    }
    std::sort(keyOrders.begin(), keyOrders.end());
  }

  bool operator<(const SinkOutputData& other) const {
    return weight > other.weight;
  }

  const SinkProfile* profile;
  std::vector<SinkTypeData> arrCounts;
  std::vector<SinkTypeData> keyCounts;
  std::vector<SinkTypeData> valCounts;
  std::vector<SourceFrequencyData> sources;
  std::vector<KeyOrderOutputData> keyOrders;
  uint64_t loggedCount = 0;
  uint64_t sampledCount = 0;
  uint64_t unsampledCount = 0;
  uint64_t weight = 0;
};

using ProfileOutputData = std::vector<SourceOutputData>;
using ProfileMap = tbb::concurrent_hash_map<LoggingProfileKey, LoggingProfile*,
                                            LoggingProfileKey::TbbHashCompare>;

using SinkProfileKeyHashCompare = pairHashCompare<
  TransID, SrcKey, integralHashCompare<TransID>, SrcKey::TbbHashCompare>;
using SinkMap = tbb::concurrent_hash_map<
  SinkProfileKey, SinkProfile*, SinkProfileKeyHashCompare>;

ProfileMap s_profileMap;
SinkMap s_sinkMap;
std::thread s_exportProfilesThread;

template<typename... Ts>
bool logToFile(FILE* file, Ts&&... args) {
  auto const entry = folly::sformat(std::forward<Ts>(args)...);
  return fwrite(entry.data(), 1, entry.length(), file) == entry.length();
}

#define LOG_OR_RETURN(...) if (!logToFile(__VA_ARGS__)) return false;

bool exportOperationSet(FILE* file,
                        const std::vector<OperationOutputData>& operations) {
  for (auto const& operationData : operations) {
    if (operationData.events.size() == 1) {
      // There's only one distinct event for this op, print it at this level.
      auto const eventData = *operationData.events.begin();
      assertx(operationData.totalCount == eventData.count);
      LOG_OR_RETURN(file, "  {: >6}x {}\n", eventData.count,
                    eventData.event.toString());
      continue;
    }

    LOG_OR_RETURN(file, "  {: >6}x {}\n", operationData.totalCount,
                  arrayOpToString(operationData.operation));

    for (auto const& eventData : operationData.events) {
      LOG_OR_RETURN(file, "        {: >6}x {}\n", eventData.count,
                    eventData.event.toString());
    }
  }

  return true;
}

bool exportTypeCounts(FILE* file, const char* label,
                      const std::vector<SinkTypeData>& counts) {
  LOG_OR_RETURN(file, "  {} Type Counts:\n", label);
  for (auto const& count : counts) {
    LOG_OR_RETURN(file, "  {: >6}x {}\n", count.count, count.name);
  }
  return true;
}

bool exportSortedProfiles(FILE* file, const ProfileOutputData& profileData) {
  LOG_OR_RETURN(file, "========================================================================\n");
  LOG_OR_RETURN(file, "Sources:\n\n");

  for (auto const& sourceData : profileData) {
    auto const sourceProfile = sourceData.profile;
    assertx(sourceProfile->data);

    LOG_OR_RETURN(file, "{} [{}/{} sampled, {:.2f} weight]\n",
                  sourceProfile->key.toString(),
                  sourceProfile->data->loggingArraysEmitted.load(),
                  sourceProfile->data->sampleCount.load(), sourceData.weight);
    LOG_OR_RETURN(file, "  {}\n", sourceProfile->key.toStringDetail());
    LOG_OR_RETURN(file, "  Selected Layout: {}\n",
                  sourceProfile->getLayout().describe());
    LOG_OR_RETURN(file, "  {} reads, {} writes\n",
                  sourceData.readCount, sourceData.writeCount);

    LOG_OR_RETURN(file, "  Read operations:\n");
    if (!exportOperationSet(file, sourceData.readOperations)) return false;

    LOG_OR_RETURN(file, "  Write operations:\n");
    if (!exportOperationSet(file, sourceData.writeOperations)) return false;

    LOG_OR_RETURN(file, "  Entry Type Escalations:\n");
    for (auto const& escData : sourceData.monotypeEscalations) {
      LOG_OR_RETURN(file, "  {: >6}x {} -> {}\n", escData.count,
                    escData.before.toString(), escData.after.toString());
    }

    LOG_OR_RETURN(file, "  Entry Type Operations:\n");
    for (auto const& useData : sourceData.monotypeUses) {
      LOG_OR_RETURN(file, "  {: >6}x {}\n", useData.count,
                    useData.state.toString());
    }

    LOG_OR_RETURN(file, "  Static String Key Insertion Order:\n");
    for (auto const koData : sourceData.keyOrders) {
      LOG_OR_RETURN(file, "  {: >6}x {}\n", koData.count, koData.ko.toString());
    }

    LOG_OR_RETURN(file, "\n");
  }

  return true;
}

bool exportSortedSinks(FILE* file, const std::vector<SinkOutputData>& sinks) {
  LOG_OR_RETURN(file, "========================================================================\n");
  LOG_OR_RETURN(file, "Sinks:\n\n");

  for (auto const& sink : sinks) {
    auto const sk = sink.profile->key.second;
    LOG_OR_RETURN(file, "{} [{}/{} sampled, {} logged]\n",
                  sk.getSymbol(), sink.sampledCount,
                  sink.weight, sink.loggedCount);
    LOG_OR_RETURN(file, "  {}\n", sk.showInst());
    LOG_OR_RETURN(file, "  Selected Layout: {}\n",
                  sink.profile->getLayout().describe());

    if (!exportTypeCounts(file, "Array", sink.arrCounts)) return false;
    if (!exportTypeCounts(file, "Key",   sink.keyCounts)) return false;
    if (!exportTypeCounts(file, "Value", sink.valCounts)) return false;

    LOG_OR_RETURN(file, "  Sources:\n");
    for (auto const& edge : sink.sources) {
      LOG_OR_RETURN(file, "  {: >6}x {}\n", edge.count, edge.source);
    }

    LOG_OR_RETURN(file, "  Static String Key Insertion Order:\n");
    for (auto const koData : sink.keyOrders) {
      LOG_OR_RETURN(file, "  {: >6}x {}\n", koData.count, koData.ko.toString());
    }

    LOG_OR_RETURN(file, "\n");
  }

  return true;
}

bool exportLayouts(FILE* file) {
  LOG_OR_RETURN(file, "========================================================================\n");
  LOG_OR_RETURN(file, "Layouts:\n\n");
  LOG_OR_RETURN(file, "{}", Layout::dumpAllLayouts());

  return true;
}

SourceOutputData sortSourceData(const LoggingProfile* profile) {
  assertx(profile->data);
  // Group events by their operation
  std::map<ArrayOp, OperationOutputData> opsGrouped;
  for (auto const& eventAndCount : profile->data->events) {
    auto const event = EventKey(eventAndCount.first);
    auto const count = eventAndCount.second;

    auto const op = event.getOp();
    auto const it = opsGrouped.try_emplace(op, op);
    it.first->second.registerEvent(event, count);
  }

  for (auto& operation : opsGrouped) {
    operation.second.formatEvents();
  }

  // Flatten to vectors
  std::vector<OperationOutputData> operations;
  operations.reserve(opsGrouped.size());
  std::transform(
    opsGrouped.begin(), opsGrouped.end(), std::back_inserter(operations),
    [](const std::pair<ArrayOp, OperationOutputData>& operationData) {
      return operationData.second;
    }
  );

  // Determine monotype operations
  std::vector<EntryTypesEscalationOutputData> escalations;
  std::map<uint16_t, uint64_t> usesMap;
  for (auto const& statesAndCount : profile->data->entryTypes) {
    auto const before = statesAndCount.first.first;
    auto const after = statesAndCount.first.second;
    auto const count = statesAndCount.second;

    if (before != after) {
      escalations.emplace_back(EntryTypes(before), EntryTypes(after), count);
    }

    usesMap[after] += count;
  }

  std::vector<EntryTypesUseOutputData> uses;
  uses.reserve(usesMap.size());
  std::transform(
    usesMap.begin(), usesMap.end(), std::back_inserter(uses),
    [](const std::pair<uint16_t, uint64_t>& useData) {
      auto const state = EntryTypes(useData.first);
      return EntryTypesUseOutputData(state, useData.second);
    }
  );

  std::vector<KeyOrderOutputData> keyOrders;
  for (auto const& keyOrderAndCount : profile->data->keyOrders) {
    keyOrders.push_back({keyOrderAndCount.first, keyOrderAndCount.second});
  }

  return SourceOutputData(profile, std::move(operations),
                          std::move(escalations), std::move(uses),
                          std::move(keyOrders));
}

ProfileOutputData sortProfileData() {
  ProfileOutputData profileData;
  profileData.reserve(s_profileMap.size());

  std::transform(
    s_profileMap.begin(), s_profileMap.end(), std::back_inserter(profileData),
    [](auto const& pair) { return sortSourceData(pair.second); }
  );
  std::sort(profileData.begin(), profileData.end());

  return profileData;
}

std::vector<SinkOutputData> sortSinkData() {
  std::vector<SinkOutputData> sinkData;
  sinkData.reserve(s_sinkMap.size());

  for (auto const& it : s_sinkMap) {
    sinkData.push_back(SinkOutputData(it.second));
  }
  std::sort(sinkData.begin(), sinkData.end());

  return sinkData;
}

}

void stopProfiling() {
  assertx(allowBespokeArrayLikes());
  assertx(s_profiling.load(std::memory_order_acquire));
  folly::SharedMutex::WriteHolder lock{s_profilingLock};
  s_profiling.store(false, std::memory_order_release);
}

namespace {
void freeProfileData() {
  if (shouldTestBespokeArrayLikes()) {
    // When testing bespoke array-likes, we use the data struct to cache
    // logging and monotype arrays. Therefore, we do not free them after layout
    // selection.
    return;
  }

  eachSource([](auto& x) { x.releaseData(); });
  eachSink([](auto& x) { x.releaseData(); });
}
}

void startExportProfiles() {
  if (RO::EvalExportLoggingArrayDataPath.empty()) {
    freeProfileData();
    return;
  }

  s_exportProfilesThread = std::thread([] {
    SCOPE_EXIT { freeProfileData(); };

    auto const sources = sortProfileData();
    auto const sinks = sortSinkData();
    auto const file = fopen(RO::EvalExportLoggingArrayDataPath.c_str(), "w");
    if (!file) return;

    SCOPE_EXIT { fclose(file); };

    exportLayouts(file);
    exportSortedProfiles(file, sources);
    exportSortedSinks(file, sinks);
  });
}

void waitOnExportProfiles() {
  if (s_exportProfilesThread.joinable()) {
    s_exportProfilesThread.join();
  }
}

//////////////////////////////////////////////////////////////////////////////

namespace {
void freeStaticArray(ArrayData* ad) {
  // TODO(T80237666): Make this code less fragile by moving this logic next
  // to the corresponding allocation logic in HashTable::staticAlloc().
  assertx(ad->isStatic());
  auto const extra = ad->hasStrKeyTable()
    ? (sizeof(StrKeyTable) + 15) & ~15ull
    : 0;
  auto const alloc = reinterpret_cast<char*>(ad) - extra;
  RO::EvalLowStaticArrays ? low_free(alloc) : uncounted_free(alloc);
}

bool shouldLogAtSrcKey(SrcKey sk) {
  if (!sk.valid()) {
    FTRACE(5, "VMRegAnchor failed for maybeMakeLoggingArray.\n");
    return false;
  }

  // Don't profile static dicts used for TypeStruct tests. Rather than using
  // these dicts, we almost always just do a DataType check on the value.
  if (sk.op() == Op::Dict && sk.advanced().op() == Op::IsTypeStructC) {
    FTRACE(5, "Skipping static array used for TypeStruct test.\n");
    return false;
  }

  return true;
}

LoggingProfile* getLoggingProfile(LoggingProfileKey key) {
  assertx(allowBespokeArrayLikes());
  {
    ProfileMap::const_accessor it;
    if (s_profileMap.find(it, key)) return it->second;
  }

  // Hold the read mutex for the duration of the mutation so that export
  // cannot begin until the mutation is complete.
  folly::SharedMutex::ReadHolder lock{s_profilingLock};
  if (!s_profiling.load(std::memory_order_acquire)) return nullptr;

  auto profile = std::make_unique<LoggingProfile>(key);
  auto const ad = getStaticArray(key);
  if (ad) {
    profile->data->staticLoggingArray = LoggingArray::MakeStatic(ad, profile.get());
    profile->data->staticSampledArray = ad->makeSampledStaticArray();
  }

  auto const result = [&]{
    ProfileMap::accessor insert;
    if (s_profileMap.insert(insert, key)) {
      insert->second = profile.release();
    }
    return insert->second;
  }();

  // If the array was static, we must either log the new static memory we used
  // or free that memory, depending on whether we won the race to set profile.
  if (ad) {
    if (profile) {
      freeStaticArray(profile->data->staticSampledArray);
      freeStaticArray(profile->data->staticLoggingArray);
    } else {
      MemoryStats::LogAlloc(AllocKind::StaticArray, sizeof(LoggingArray));
      MemoryStats::LogAlloc(AllocKind::StaticArray, allocSize(ad));
    }
  }
  return result;
}
}

LoggingProfile* getLoggingProfile(SrcKey sk) {
  if (!shouldLogAtSrcKey(sk)) return nullptr;
  return getLoggingProfile(LoggingProfileKey(canonicalize(sk)));
}

LoggingProfile* getLoggingProfile(RuntimeStruct* runtimeStruct) {
  auto const profile = runtimeStruct->m_profile.load(std::memory_order_relaxed);
  if (profile) return profile;
  auto const newProfile = getLoggingProfile(LoggingProfileKey(runtimeStruct));
  runtimeStruct->m_profile.store(newProfile, std::memory_order_relaxed);
  return newProfile;
}

LoggingProfile* getLoggingProfile(const Class* cls, Slot slot) {
  if (cls->declProperties()[slot].name == s_86reified_prop.get()) {
    return nullptr;
  }
  return getLoggingProfile(LoggingProfileKey(cls, slot));
}

SinkProfile* getSinkProfile(TransID id, SrcKey skRaw) {
  assertx(allowBespokeArrayLikes());
  auto const key = SinkProfileKey { id, canonicalize(skRaw) };
  {
    SinkMap::const_accessor it;
    if (s_sinkMap.find(it, key)) return it->second;
  }

  // Hold the read mutex for the duration of the mutation so that export cannot
  // begin until the mutation is complete.
  folly::SharedMutex::ReadHolder lock{s_profilingLock};
  if (!s_profiling.load(std::memory_order_acquire)) return nullptr;

  auto profile = std::make_unique<SinkProfile>(key);

  auto const result = [&]{
    SinkMap::accessor insert;
    if (s_sinkMap.insert(insert, key)) {
      insert->second = profile.release();
    }
    return insert->second;
  }();

  // Don't hold the lock while we free profile.
  return result;
}

SrcKey getSrcKey() {
  // If there are no VM frames, don't drop an anchor
  if (!rds::header()) return SrcKey();

  VMRegAnchor _(VMRegAnchor::Soft);
  if (tl_regState != VMRegState::CLEAN || vmfp() == nullptr) {
    return SrcKey();
  }
  auto const fp = vmfp();
  auto const func = fp->func();
  if (!func->contains(vmpc())) return SrcKey();

  auto const result = SrcKey(func, vmpc(), ResumeMode::None);
  assertx(canonicalize(result) == result);
  return result;
}

void eachSource(std::function<void(LoggingProfile& profile)> fn) {
  assertx(!s_profiling.load(std::memory_order_acquire));
  for (auto& it : s_profileMap) fn(*it.second);
}

void eachSink(std::function<void(SinkProfile& profile)> fn) {
  assertx(!s_profiling.load(std::memory_order_acquire));
  for (auto& it : s_sinkMap) fn(*it.second);
}

void deserializeSource(LoggingProfileKey key, jit::ArrayLayout layout) {
  ProfileMap::accessor insert;
  always_assert(s_profileMap.insert(insert, key));
  insert->second = new LoggingProfile(key, layout);
}

void deserializeSink(SinkProfileKey key, jit::ArrayLayout layout) {
  SinkMap::accessor insert;
  always_assert(s_sinkMap.insert(insert, key));
  insert->second = new SinkProfile(key, layout);
}

size_t countSources() {
  return s_profileMap.size();
}

size_t countSinks() {
  return s_sinkMap.size();
}

ArrayOp getArrayOp(uint64_t key) {
  return EventKey(key).getOp();
}

//////////////////////////////////////////////////////////////////////////////

}}
