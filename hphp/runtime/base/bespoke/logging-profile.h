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

#ifndef HPHP_LOGGING_PROFILE_H_
#define HPHP_LOGGING_PROFILE_H_

#include "hphp/runtime/base/bespoke/entry-types.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/vm/srckey.h"
#include "hphp/runtime/vm/jit/array-layout.h"

#include <folly/String.h>
#include <tbb/concurrent_hash_map.h>

#include <algorithm>
#include <atomic>

namespace HPHP { namespace bespoke {

struct LoggingArray;

// The second entry in these tuples is an "is read operation" flag.
// This flag is set for ops that are guaranteed to preserve the array's layout,
// even if - like with the ToVArray op - they may update the array due to COW.
#define ARRAY_OPS \
  X(Scan,               true)  \
  X(EscalateToVanilla,  true)  \
  X(ConvertToUncounted, true)  \
  X(ReleaseUncounted,   true)  \
  X(Release,            true)  \
  X(IsVectorData,       true)  \
  X(GetInt,             true)  \
  X(GetStr,             true)  \
  X(GetIntPos,          true)  \
  X(GetStrPos,          true)  \
  X(LvalInt,            false) \
  X(LvalStr,            false) \
  X(ElemInt,            false) \
  X(ElemStr,            false) \
  X(SetInt,             false) \
  X(SetStr,             false) \
  X(ConstructInt,       false) \
  X(ConstructStr,       false) \
  X(RemoveInt,          false) \
  X(RemoveStr,          false) \
  X(IterBegin,          true)  \
  X(IterLast,           true)  \
  X(IterEnd,            true)  \
  X(IterAdvance,        true)  \
  X(IterRewind,         true)  \
  X(Append,             false) \
  X(Pop,                false) \
  X(ToDVArray,          true)  \
  X(ToHackArr,          true)  \
  X(PreSort,            true)  \
  X(PostSort,           true)  \
  X(SetLegacyArray,     true)

enum class ArrayOp : uint8_t {
#define X(name, read) name,
ARRAY_OPS
#undef X
};

// Internal storage detail of EventMap.
struct EventKey;

// We profile some bytecodes (array constructors or casts) and prop init vals.
struct LoggingProfileKey {
  struct TbbHashCompare;

  explicit LoggingProfileKey(SrcKey sk) : sk(sk), slot(kInvalidSlot) {}
  explicit LoggingProfileKey(const Class* cls, Slot slot)
    : cls(cls), slot(slot) {}

  Op op() const {
    return slot == kInvalidSlot ? sk.op() : Op::NewObjD;
  }

  std::string toString() const {
    if (slot == kInvalidSlot) return sk.getSymbol();
    auto const& prop = cls->declProperties()[slot];
    return folly::sformat("{}->{}", cls->name(), prop.name);
  }

  std::string toStringDetail() const {
    if (slot == kInvalidSlot) return sk.showInst();
    return folly::sformat("NewObjD \"{}\"", cls->name());
  }

  union {
    SrcKey sk;
    const Class* cls;
    uintptr_t ptr;
  };
  // The logical slot of a property on cls, or kInvalidSlot if sk is set.
  Slot slot;
};

struct LoggingProfileKey::TbbHashCompare {
  static size_t hash(const LoggingProfileKey& key) {
    return folly::hash::hash_combine(hash_int64(key.ptr), key.slot);
  }
  static bool equal(const LoggingProfileKey& a, const LoggingProfileKey& b) {
    return a.ptr == b.ptr && a.slot == b.slot;
  }
};

// We'll store a LoggingProfile for each array construction site SrcKey.
// It tracks the operations that happen on arrays coming from that site.
struct LoggingProfile {
  // Values in the event map are sampled event counts.
  using EventMap = tbb::concurrent_hash_map<uint64_t, size_t,
                                            integralHashCompare<uint64_t>>;

  // The first element of the key is the EntryTypes before the operation;
  // the second element is the EntryTypes after it.
  using EntryTypesMapKey = std::pair<uint16_t, uint16_t>;
  using EntryTypesMapHasher = pairHashCompare<uint16_t, uint16_t,
                                              integralHashCompare<uint16_t>,
                                              integralHashCompare<uint16_t>>;
  using EntryTypesMap = tbb::concurrent_hash_map<EntryTypesMapKey, size_t,
                                                 EntryTypesMapHasher>;

  explicit LoggingProfile(LoggingProfileKey key) : key(key) {}

  double getSampleCountMultiplier() const;
  uint64_t getTotalEvents() const;
  double getProfileWeight() const;

  // We take specific inputs rather than templated inputs because we're going
  // to follow up soon with limitations on the number of arguments we can log.
  void logEvent(ArrayOp op);
  void logEvent(ArrayOp op, int64_t k);
  void logEvent(ArrayOp op, const StringData* k);
  void logEvent(ArrayOp op, TypedValue v);
  void logEvent(ArrayOp op, int64_t k, TypedValue v);
  void logEvent(ArrayOp op, const StringData* k, TypedValue v);

  void logEntryTypes(EntryTypes before, EntryTypes after);

private:
  void logEventImpl(const EventKey& key);

public:
  LoggingProfileKey key;

  std::atomic<uint64_t> sampleCount = 0;
  std::atomic<uint64_t> loggingArraysEmitted = 0;
  BespokeArray* staticBespokeArray = nullptr;
  LoggingArray* staticLoggingArray = nullptr;
  ArrayData* staticSampledArray = nullptr;
  EventMap events;
  EntryTypesMap entryTypes;

  jit::ArrayLayout layout = jit::ArrayLayout::Bottom();
};

// We split sinks by profiling tracelet so we can condition on array type.
using SinkProfileKey = std::pair<TransID, SrcKey>;

// We'll store a SinkProfile for each place where an array is used.
struct SinkProfile {
  using SourceMap = tbb::concurrent_hash_map<LoggingProfile*, size_t,
                                             pointer_hash<LoggingProfile>>;

  void reduce(const SinkProfile& other);
  void update(const ArrayData* ad);

public:
  static constexpr size_t kNumArrTypes = ArrayData::kNumKinds / 2;
  static constexpr size_t kNumKeyTypes = int(KeyTypes::Any) + 1;
  static constexpr size_t kNumValTypes = kMaxDataType - kMinDataType + 3;

  static constexpr size_t kNoValTypes = kNumValTypes - 2;
  static constexpr size_t kAnyValType = kNumValTypes - 1;

  SinkProfileKey key;

  std::atomic<uint64_t> arrCounts[kNumArrTypes] = {};
  std::atomic<uint64_t> keyCounts[kNumKeyTypes] = {};
  std::atomic<uint64_t> valCounts[kNumValTypes] = {};

  std::atomic<uint64_t> sampledCount = 0;
  std::atomic<uint64_t> unsampledCount = 0;
  SourceMap sources;

  jit::ArrayLayout layout = jit::ArrayLayout::Bottom();
};

// Return a profile for the given (valid) SrcKey. If no profile for the SrcKey
// exists, a new one is made. If we're done profiling or it's not useful to
// profile this bytecode, this function will return nullptr.
LoggingProfile* getLoggingProfile(SrcKey sk);
LoggingProfile* getLoggingProfile(const Class* cls, Slot slot);

// Return a profile for the given profiling tracelet and (valid) sink SrcKey.
// If no profile for the sink exists, a new one is made. May return nullptr.
SinkProfile* getSinkProfile(TransID id, SrcKey sk);

// Attempt to get the current SrcKey. May fail and return an invalid SrcKey.
SrcKey getSrcKey();

// Hooks called by layout selection at the appropriate times.
void stopProfiling();
void startExportProfiles();

// Global views, used for layout selection.
void eachSource(std::function<void(LoggingProfile& profile)> fn);
void eachSink(std::function<void(SinkProfile& profile)> fn);

// Accessors for logged events. TODO(kshaunak): Expose a better API.
ArrayOp getArrayOp(uint64_t key);

}}

#endif // HPHP_LOGGING_PROFILE_H_
