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
}

//////////////////////////////////////////////////////////////////////////////

// The key for a sampled event. The granularity we choose is important here:
// too fine, and our profiles will have too many entries; too coarse, and we
// won't be able to make certain optimizations.
struct alignas(8) EventKey {
  EventKey(ArrayOp op);
  EventKey(ArrayOp op, int64_t k);
  EventKey(ArrayOp op, const StringData* k);
  EventKey(ArrayOp op, TypedValue v);
  EventKey(ArrayOp op, int64_t k, TypedValue v);
  EventKey(ArrayOp op, const StringData* k, TypedValue v);

  EventKey(uint64_t value) {
    static_assert(sizeof(EventKey) == sizeof(uint64_t), "");
    memmove(this, &value, sizeof(EventKey));
  }
  uint64_t toUInt64() const {
    uint64_t value;
    memmove(&value, this, sizeof(EventKey));
    return value;
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
  auto const op = [&]{
    switch (m_op) {
#define X(name) case ArrayOp::name: return #name;
ARRAY_OPS
#undef X
    }
    not_reached();
  }();
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

size_t LoggingProfile::eventCount() const {
  auto result = size_t{0};
  for (auto const& pair : events) result += pair.second;
  return result;
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
  auto const sink = getSrcKey();
  if (events.insert(it, {sink, key.toUInt64()})) {
    it->second = 1;
  } else {
    it->second++;
  }
  FTRACE(6, "{} -> {}: {} [count={}]\n", source.getSymbol(),
         (sink.valid() ? sink.getSymbol() : "<unknown>"),
         EventKey(key).toString(), it->second);
}

//////////////////////////////////////////////////////////////////////////////

namespace {

using EventData = std::pair<LoggingProfile::EventMapKey, size_t>;
using SrcKeyData = std::pair<SrcKey, std::vector<EventData>>;
using ProfileMap = tbb::concurrent_hash_map<SrcKey, LoggingProfile*,
                                            SrcKey::TbbHashCompare>;

ProfileMap s_profileMap;
std::thread s_exportProfilesThread;

void exportSortedProfiles(FILE* file, const std::vector<SrcKeyData>& sources) {
  for (auto const& sourceProfile : sources) {
    auto const sourceSk = sourceProfile.first;

    if (RO::EvalExportLoggingArrayDataToFile) {
      auto const logEntry = folly::sformat("{}:\n", sourceSk.getSymbol());
      if (fwrite(logEntry.data(), 1, logEntry.length(), file)
          != logEntry.length()) {
        return;
      }
    }

    for (auto const& entry : sourceProfile.second) {
      auto const sinkSk = entry.first.first;
      auto const event = entry.first.second;
      auto const count = entry.second;

      if (RO::EvalExportLoggingArrayDataToFile) {
        auto const logEntry =
          folly::sformat("  {: >9}x {} at {}\n", count,
                         EventKey(event).toString(),
                         sinkSk.valid() ? sinkSk.getSymbol() : "<unknown>");

        if (fwrite(logEntry.data(), 1, logEntry.length(), file)
            != logEntry.length()) {
          return;
        }
      }

      if (RO::EvalExportLoggingArrayDataToStructuredLog) {
        StructuredLogEntry sle;
        sle.setStr("source", sourceSk.getSymbol());
        sle.setStr("sink", sinkSk.valid() ? sinkSk.getSymbol() : "<unknown>");
        sle.setStr("event", EventKey(event).toString());
        sle.setInt("count", count);

        // TODO: Use a permament log category when our format is stabilized.
        StructuredLog::log("hhvm_classypgo_array_logs_test", sle);
      }
    }
  }
}

std::vector<SrcKeyData> sortProfileData() {
  using OriginalData = std::pair<SrcKey, LoggingProfile*>;
  using OriginalDataWithWeight = std::pair<OriginalData, size_t>;
  std::vector<OriginalDataWithWeight> presortVec;
  presortVec.reserve(s_profileMap.size());

  std::transform(
    s_profileMap.begin(), s_profileMap.end(), std::back_inserter(presortVec),
    [](OriginalData data) {
      return std::make_pair(data, data.second->eventCount());
    }
  );

  std::sort(
    presortVec.begin(), presortVec.end(),
    [](const OriginalDataWithWeight& a,
        const OriginalDataWithWeight& b) {
      return a.second > b.second;
    }
  );

  std::vector<SrcKeyData> sources;
  sources.reserve(s_profileMap.size());

  std::transform(
    presortVec.begin(), presortVec.end(), std::back_inserter(sources),
    [](const OriginalDataWithWeight& srcKeyProfilePair) {
      auto const srcKeyProfile = srcKeyProfilePair.first;
      std::vector<EventData> events(srcKeyProfile.second->events.begin(),
                                    srcKeyProfile.second->events.end());

      std::sort(
        events.begin(),
        events.end(),
        [&](EventData a, EventData b) {
          return a.second > b.second;
        }
      );

      return std::make_pair(srcKeyProfile.first, events);
    }
  );

  return sources;
}

}

void exportProfiles() {
  if (!RO::EvalExportLoggingArrayDataToFile &&
      !RO::EvalExportLoggingArrayDataToStructuredLog) {
    return;
  }

  {
    folly::SharedMutex::WriteHolder lock{s_exportStartedLock};
    s_exportStarted.store(true, std::memory_order_relaxed);
  }

  s_exportProfilesThread = std::thread([] {
    hphp_thread_init();

    // TODO: Not thread safe! We can't do any global operations on these
    // concurrent hash maps when writers may write to them as well, and there
    // may be LoggingArrays in APC even after we stop emitting them.
    //
    // Instead, we can guard profile creation and logEvent with an RWLock;
    // "readers" can insert entries, and "writers" can swap out the map.
    auto const sources = sortProfileData();
    auto const file = [&]() -> FILE* {
      if (!RO::EvalExportLoggingArrayDataToFile) return nullptr;
      auto const filename = folly::to<std::string>(
        RO::EvalExportLoggingArrayDataPath, "/logging_array_export");
      return fopen(filename.c_str(), "w");
    }();
    SCOPE_EXIT { if (file) fclose(file); };

    exportSortedProfiles(file, sources);
  });
}

void waitOnExportProfiles() {
  if (s_exportStarted.load(std::memory_order_relaxed)) {
    s_exportProfilesThread.join();
  }
}

//////////////////////////////////////////////////////////////////////////////

LoggingProfile* getLoggingProfile(SrcKey sk, ArrayData *ad) {
  {
    ProfileMap::const_accessor it;
    if (s_profileMap.find(it, sk)) return it->second;
  }

  // Hold the read mutex for the duration of the mutation so that export cannot
  // begin until the mutation is complete.
  folly::SharedMutex::ReadHolder lock{s_exportStartedLock};
  if (s_exportStarted.load(std::memory_order_relaxed)) return nullptr;

  auto prof = std::make_unique<LoggingProfile>(sk);
  if (ad->isStatic()) {
    prof->staticArray = LoggingArray::MakeStatic(ad, prof.get());
  }

  ProfileMap::accessor insert;
  if (s_profileMap.insert(insert, sk)) {
    insert->second = prof.release();
    MemoryStats::LogAlloc(AllocKind::StaticArray, sizeof(LoggingArray));
  } else {
    // Someone beat us; clean up
    if (ad->isStatic()) {
      LoggingArray::FreeStatic(prof->staticArray);
    }
  }

  return insert->second;
}

SrcKey getSrcKey() {
  VMRegAnchor _(VMRegAnchor::Soft);
  if (tl_regState != VMRegState::CLEAN || vmfp() == nullptr) {
    return SrcKey();
  }
  auto const fp = vmfp();
  return SrcKey(vmfp()->func(), vmpc(), resumeModeFromActRec(fp));
}

//////////////////////////////////////////////////////////////////////////////

}}
