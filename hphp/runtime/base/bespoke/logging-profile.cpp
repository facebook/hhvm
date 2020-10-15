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

#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/bespoke/logging-array.h"
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

folly::SharedMutex s_exportStartedLock;
std::atomic<bool> s_exportStarted{false};

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
  uint32_t m_key = 0; // Set for Spec::Int8 and Spec::Str32
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
    if (spec == Spec::Int8) {
      auto const i = safe_cast<int8_t>(safe_cast<int16_t>(m_key) + kInt8Min);
      return folly::sformat(" key=[i8:{}]", i);
    }
    if (spec == Spec::Str32) {
      auto const s = ((StringData*)safe_cast<uintptr_t>(m_key))->data();
      return folly::sformat(" key=[s32:\"{}\"]", folly::cEscape<std::string>(s));
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

double LoggingProfile::getSampleCountMultiplier() const {
  if (loggingArraysEmitted == 0) return 0;

  return sampleCount / (double) loggingArraysEmitted;
}

uint64_t LoggingProfile::getTotalEvents() const {
  size_t total = 0;
  for (auto const& event : events) total += event.second;

  return total;
}

double LoggingProfile::getProfileWeight() const {
  return getTotalEvents() * getSampleCountMultiplier();
}

void LoggingProfile::logReach(TransID transId, uint32_t guardIdx) {
  // Hold the read mutex for the duration of the mutation so that export cannot
  // begin until the mutation is complete.
  folly::SharedMutex::ReadHolder lock{s_exportStartedLock};
  if (s_exportStarted.load(std::memory_order_relaxed)) return;

  ReachMap::accessor it;
  if (reachedTracelets.insert(it, {transId, guardIdx})) {
    it->second = 1;
  } else {
    it->second++;
  }
  FTRACE(6, "{} reached {}, guard {} [count={}]\n", source.getSymbol(),
         transId, guardIdx, it->second);
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
  // Hold the read mutex for the duration of the mutation so that export cannot
  // begin until the mutation is complete.
  folly::SharedMutex::ReadHolder lock{s_exportStartedLock};
  if (s_exportStarted.load(std::memory_order_relaxed)) return;

  EventMap::accessor it;
  auto const has_sink = key.getOp() != ArrayOp::ReleaseUncounted;
  auto const sink = has_sink ? getSrcKey() : SrcKey{};
  if (events.insert(it, {sink, key.toUInt64()})) {
    it->second = 1;
  } else {
    it->second++;
  }
  FTRACE(6, "{} -> {}: {} [count={}]\n", source.getSymbol(),
         (sink.valid() ? sink.getSymbol() : "<unknown>"),
         EventKey(key).toString(), it->second);
}

void LoggingProfile::logEntryTypes(EntryTypes before, EntryTypes after) {
  // Hold the read mutex for the duration of the mutation so that export cannot
  // begin until the mutation is complete.
  folly::SharedMutex::ReadHolder lock{s_exportStartedLock};
  if (s_exportStarted.load(std::memory_order_relaxed)) return;

  EntryTypesMap::accessor it;
  if (monotypeEvents.insert(it, {before.asInt16(), after.asInt16()})) {
    it->second = 1;
  } else {
    it->second++;
  }

  FTRACE(6, "EntryTypes escalation {} -> {} [count={}]\n", before.toString(),
         after.toString(), it->second);
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

  void sortEvents() {
    std::sort(events.begin(), events.end());
  }

  ArrayOp operation;
  std::vector<EventOutputData> events;
  uint64_t totalCount;
};

struct SinkOutputData {
  SinkOutputData(SrcKey sink, uint64_t totalCount)
    : sink(sink)
    , totalCount(totalCount)
  {}

  bool operator<(const SinkOutputData& other) const {
    return totalCount > other.totalCount;
  }

  SrcKey sink;
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

struct TraceletReachOutputData {
  TraceletReachOutputData(TransID tid, size_t guardIdx, uint64_t count)
    : tid(tid)
    , guardIdx(guardIdx)
    , count(count)
  {}

  bool operator<(const TraceletReachOutputData& other) const {
    return count > other.count;
  }

  TransID tid;
  size_t guardIdx;
  uint64_t count;
};

struct SourceOutputData {
  SourceOutputData(const LoggingProfile* profile,
                   std::vector<OperationOutputData>&& operations,
                   std::vector<SinkOutputData>&& sinkData,
                   std::vector<EntryTypesEscalationOutputData>&& monoEscalations,
                   std::vector<EntryTypesUseOutputData>&& monoUses,
                   std::vector<TraceletReachOutputData>&& traceletReachData)
    : profile(profile)
    , monotypeEscalations(std::move(monoEscalations))
    , monotypeUses(std::move(monoUses))
    , sinks(std::move(sinkData))
    , traceletReach(std::move(traceletReachData))
  {
    std::sort(sinks.begin(), sinks.end());
    std::sort(monotypeEscalations.begin(), monotypeEscalations.end());
    std::sort(monotypeUses.begin(), monotypeUses.end());
    std::sort(operations.begin(), operations.end());
    std::sort(traceletReach.begin(), traceletReach.end());

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
  std::vector<SinkOutputData> sinks;
  std::vector<TraceletReachOutputData> traceletReach;
  uint64_t readCount;
  uint64_t writeCount;
  double weight;
};

using ProfileOutputData = std::vector<SourceOutputData>;
using ProfileMap = tbb::concurrent_hash_map<SrcKey, LoggingProfile*,
                                            SrcKey::TbbHashCompare>;

ProfileMap s_profileMap;
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

bool exportSortedProfiles(FILE* file, const ProfileOutputData& profileData) {
  for (auto const& sourceData : profileData) {
    auto const sourceProfile = sourceData.profile;
    auto const sourceSk = sourceProfile->source;

    LOG_OR_RETURN(file, "{} [{}/{} sampled, {:.2f} weight]\n",
                  sourceSk.getSymbol(),
                  sourceProfile->loggingArraysEmitted.load(),
                  sourceProfile->sampleCount.load(), sourceData.weight);
    LOG_OR_RETURN(file, "  {}\n", sourceSk.showInst());
    LOG_OR_RETURN(file, "  {} reads, {} writes, {} distinct sinks\n",
                  sourceData.readCount, sourceData.writeCount,
                  sourceData.sinks.size());

    LOG_OR_RETURN(file, "  Read operations:\n");
    if (!exportOperationSet(file, sourceData.readOperations)) return false;

    LOG_OR_RETURN(file, "  Write operations:\n");
    if (!exportOperationSet(file, sourceData.writeOperations)) return false;

    LOG_OR_RETURN(file, "  Sinks:\n");
    for (auto const& sinkData : sourceData.sinks) {
      LOG_OR_RETURN(file, "  {: >6}x {}\n", sinkData.totalCount,
                    sinkData.sink.valid() ? sinkData.sink.getSymbol()
                                          : "<unknown>");
    }

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

    LOG_OR_RETURN(file, "  Reached tracelets:\n");
    for (auto const& traceletData : sourceData.traceletReach) {
      LOG_OR_RETURN(file, "  {: >6}x {}, guard {}\n", traceletData.count,
                    traceletData.tid, traceletData.guardIdx);
    }

    LOG_OR_RETURN(file, "\n");
  }

  return true;
}

SourceOutputData sortSourceData(const LoggingProfile* profile) {
  // Aggregate total events by event key
  std::map<uint64_t, uint64_t> eventCounts;
  std::map<SrcKey, uint64_t> sinkCounts;
  for (auto const& eventRecord : profile->events) {
    auto const count = eventRecord.second;
    auto const eventKey = eventRecord.first.second;

    eventCounts[eventKey] += count;
    sinkCounts[eventRecord.first.first] += count;
  }

  // Group events by their operation
  std::map<ArrayOp, OperationOutputData> opsGrouped;
  for (auto const& eventAndCount : eventCounts) {
    auto const event = EventKey(eventAndCount.first);
    auto const count = eventAndCount.second;

    auto const op = event.getOp();
    auto const it = opsGrouped.try_emplace(op, op);
    it.first->second.registerEvent(event, count);
  }

  for (auto& operation : opsGrouped) {
    operation.second.sortEvents();
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

  std::vector<SinkOutputData> sinks;
  sinks.reserve(sinkCounts.size());
  std::transform(
    sinkCounts.begin(), sinkCounts.end(), std::back_inserter(sinks),
    [](const std::pair<SrcKey, uint64_t>& sinkData) {
      return SinkOutputData(sinkData.first, sinkData.second);
    }
  );

  // Determine monotype operations
  std::vector<EntryTypesEscalationOutputData> escalations;
  std::map<uint16_t, uint64_t> usesMap;
  for (auto const& statesAndCount : profile->monotypeEvents) {
    auto const before = statesAndCount.first.first;
    auto const after = statesAndCount.first.second;
    auto const count = statesAndCount.second;

    if (before != after) {
      escalations.emplace_back(EntryTypes(before), EntryTypes(after),
                               count);
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

  std::vector<TraceletReachOutputData> reaches;
  reaches.reserve(profile->reachedTracelets.size());
  std::transform(
    profile->reachedTracelets.begin(), profile->reachedTracelets.end(),
    std::back_inserter(reaches),
    [](const std::pair<LoggingProfile::ReachLocation, size_t>& reachData) {
      return TraceletReachOutputData(reachData.first.first,
                                     reachData.first.second,
                                     reachData.second);
    }
  );

  return SourceOutputData(profile, std::move(operations), std::move(sinks),
                          std::move(escalations), std::move(uses),
                          std::move(reaches));
}

ProfileOutputData sortProfileData() {
  ProfileOutputData profileData;
  profileData.reserve(s_profileMap.size());

  // Process each profile, then sort the results
  std::transform(
    s_profileMap.begin(), s_profileMap.end(), std::back_inserter(profileData),
    [](const std::pair<SrcKey, LoggingProfile*>& sourceData) {
      return sortSourceData(sourceData.second);
    }
  );

  std::sort(profileData.begin(), profileData.end());

  return profileData;
}

}

void exportProfiles() {
  assertx(allowBespokeArrayLikes());

  if (RO::EvalExportLoggingArrayDataPath.empty()) return;

  {
    folly::SharedMutex::WriteHolder lock{s_exportStartedLock};
    s_exportStarted.store(true, std::memory_order_relaxed);
  }

  s_exportProfilesThread = std::thread([] {
    auto const sources = sortProfileData();
    auto const file = fopen(RO::EvalExportLoggingArrayDataPath.c_str(), "w");
    if (!file) return;

    SCOPE_EXIT { fclose(file); };

    exportSortedProfiles(file, sources);
  });
}

void waitOnExportProfiles() {
  if (s_exportStarted.load(std::memory_order_relaxed)) {
    s_exportProfilesThread.join();
  }
}

//////////////////////////////////////////////////////////////////////////////

LoggingProfile* getLoggingProfile(SrcKey sk, ArrayData* ad) {
  {
    ProfileMap::const_accessor it;
    if (s_profileMap.find(it, sk)) return it->second;
  }

  // Hold the read mutex for the duration of the mutation so that export cannot
  // begin until the mutation is complete.
  folly::SharedMutex::ReadHolder lock{s_exportStartedLock};
  if (s_exportStarted.load(std::memory_order_relaxed)) return nullptr;

  auto prof = std::make_unique<LoggingProfile>(sk);
  if (ad) prof->staticArray = LoggingArray::MakeStatic(ad, prof.get());

  ProfileMap::accessor insert;
  if (s_profileMap.insert(insert, sk)) {
    insert->second = prof.release();
    MemoryStats::LogAlloc(AllocKind::StaticArray, sizeof(LoggingArray));
  } else if (ad) {
    LoggingArray::FreeStatic(prof->staticArray);
  }

  return insert->second;
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
  return SrcKey(func, vmpc(), resumeModeFromActRec(fp));
}

//////////////////////////////////////////////////////////////////////////////

}}
