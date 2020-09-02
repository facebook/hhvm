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

#include "hphp/runtime/vm/srckey.h"

#include "hphp/runtime/base/program-functions.h"

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
  X(Prepend,            false) \
  X(Merge,              true)  \
  X(Pop,                false) \
  X(Dequeue,            false) \
  X(Renumber,           false) \
  X(Copy,               true)  \
  X(ToVArray,           true)  \
  X(ToDArray,           true)  \
  X(ToVec,              true)  \
  X(ToDict,             true)  \
  X(ToKeyset,           true)

enum class ArrayOp : uint8_t {
#define X(name, read) name,
ARRAY_OPS
#undef X
};

// Internal storage detail of EventMap.
struct EventKey;

// We'll store a LoggingProfile for each array construction site SrcKey.
// It tracks the operations that happen on arrays coming from that site.
struct LoggingProfile {
  using EventMapKey = std::pair<SrcKey, uint64_t>;
  using EventMapHasher = pairHashCompare<
    SrcKey, uint64_t, SrcKey::TbbHashCompare, integralHashCompare<uint64_t>>;

  // Values in the event map are sampled event counts.
  using EventMap = tbb::concurrent_hash_map<
    EventMapKey, size_t, EventMapHasher>;

  explicit LoggingProfile(SrcKey source) : source(source) {}

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

private:
  void logEventImpl(const EventKey& key);

public:
  SrcKey source;
  std::atomic<uint64_t> sampleCount = 0;
  std::atomic<uint64_t> loggingArraysEmitted = 0;
  LoggingArray* staticArray = nullptr;
  EventMap events;
};

// Return a profile for the given (valid) SrcKey. If no profile for the SrcKey
// exists, a new one is created. *StaticLoggingArray helpers are used to
// construct the shared LoggingArray if the ArrayData is static. May return
// null if no profile exists and the the profile export has begun.
LoggingProfile* getLoggingProfile(SrcKey sk, ArrayData* ad);

// Attempt to get the current SrcKey. May fail and return an invalid SrcKey.
SrcKey getSrcKey();

}}

#endif // HPHP_LOGGING_PROFILE_H_
