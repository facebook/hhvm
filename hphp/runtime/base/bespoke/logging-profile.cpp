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

//////////////////////////////////////////////////////////////////////////////

folly::SharedMutex s_exportStartedLock;
std::atomic<bool> s_exportStarted{false};

std::string toStringForKey(int64_t v) {
  if (v < 256) {
    return folly::sformat("[int={}]", v);
  } else {
    return "[int>=256]";
  }
}

std::string toStringForKey(const StringData* sd) {
  if (sd->isStatic()) {
    return folly::sformat("[static string=\"{}\"]",
                          folly::cEscape<std::string>(sd->data()));
  } else {
    return "[non-static string]";
  }
}

std::string toStringForKey(const TypedValue& tv) {
  auto const tvname = tname(tv.type());
  switch (tv.type()) {
    case KindOfBoolean:
    case KindOfInt64:
      if (tv.val().num < 256) {
        return folly::sformat("[{}={}]", tvname, tv.val().num);
      } else {
        return folly::sformat("[{}>=256]", tvname);
      }
    case KindOfPersistentString:
    case KindOfString:
      if (tv.val().pstr->isStatic()) {
        auto const escapedString =
          folly::cEscape<std::string>(tv.val().pstr->data());
        return folly::sformat("[{}, static=\"{}\"]", tvname, escapedString);
      } else {
        return folly::sformat("[{}, non-static]", tvname);
      }
    default:
      return folly::sformat("[{}]", tvname);
  }
}

std::string assembleKey(ArrayOp op) {
  switch (op) {
#define X(name) case ArrayOp::name: return #name;
ARRAY_OPS
#undef X
  }
  always_assert(false);
}

template <typename T, typename... Ts>
std::string assembleKey(ArrayOp op, T&& arg, Ts&&... args) {
  auto const paramStr =
    folly::join(", ", {toStringForKey(std::forward<T>(arg)),
                       toStringForKey(std::forward<Ts>(args))...});
  return folly::sformat("{}: {}", assembleKey(op), paramStr);
}

//////////////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////////////

size_t LoggingProfile::eventCount() const {
  auto result = size_t{0};
  for (auto const& pair : events) result += pair.second;
  return result;
}

void LoggingProfile::logEvent(ArrayOp op) {
  logEventImpl(assembleKey(op));
}
void LoggingProfile::logEvent(ArrayOp op, int64_t k) {
  logEventImpl(assembleKey(op, k));
}
void LoggingProfile::logEvent(ArrayOp op, const StringData* k) {
  logEventImpl(assembleKey(op, k));
}
void LoggingProfile::logEvent(ArrayOp op, TypedValue v) {
  logEventImpl(assembleKey(op, v));
}
void LoggingProfile::logEvent(ArrayOp op, int64_t k, TypedValue v) {
  logEventImpl(assembleKey(op, k, v));
}
void LoggingProfile::logEvent(ArrayOp op, const StringData* k, TypedValue v) {
  logEventImpl(assembleKey(op, k, v));
}

void LoggingProfile::logEventImpl(const std::string& key) {
  // Hold the read mutex for the duration of the mutation so that export cannot
  // begin until the mutation is complete.
  folly::SharedMutex::ReadHolder lock{s_exportStartedLock};
  if (s_exportStarted.load(std::memory_order_relaxed)) return;

  EventMap::accessor it;
  auto const sink = getSrcKey();
  if (events.insert(it, {sink, key})) {
    it->second = 1;
  } else {
    it->second++;
  }
  FTRACE(6, "{} -> {}: {} [count={}]\n", source.getSymbol(),
         (sink.valid() ? sink.getSymbol() : "<unknown>"), key, it->second);
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
          folly::sformat("  {: >9}x {} at {}\n", count, event,
                         sinkSk.valid() ? sinkSk.getSymbol()
                                        : "<invalid SrcKey>");

        if (fwrite(logEntry.data(), 1, logEntry.length(), file)
            != logEntry.length()) {
          return;
        }
      }

      if (RO::EvalExportLoggingArrayDataToStructuredLog) {
        StructuredLogEntry sle;
        sle.setStr("source", sourceSk.getSymbol());
        sle.setStr("sink", sinkSk.valid() ? sinkSk.getSymbol()
                                          : "<invalid SrcKey>");
        sle.setStr("event", event);
        sle.setInt("count", count);

        // TODO: Use a permament log category when our format is stabilized.
        StructuredLog::log("hhvm_classypgo_array_logs_test", sle);
      }
    }
  }
}

std::vector<SrcKeyData> sortProfileData() {
  using OriginalSrcKeyData = std::pair<SrcKey, LoggingProfile*>;
  std::vector<OriginalSrcKeyData>
    presortVec(s_profileMap.begin(), s_profileMap.end());

  // TODO: eventCount is not an O(1) op. Let's optimize further here.
  std::sort(
    presortVec.begin(),
    presortVec.end(),
    [&](OriginalSrcKeyData a, OriginalSrcKeyData b) {
      return a.second->eventCount() > b.second->eventCount();
    }
  );

  std::vector<SrcKeyData> sources;
  sources.reserve(s_profileMap.size());

  std::transform(
    presortVec.begin(), presortVec.end(), std::back_inserter(sources),
    [](OriginalSrcKeyData srcKeyProfile) {
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
