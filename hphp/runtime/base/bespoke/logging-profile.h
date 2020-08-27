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

#define ARRAY_OPS \
  X(Scan) \
  X(EscalateToVanilla) \
  X(ConvertToUncounted) \
  X(ReleaseUncounted) \
  X(Release) \
  X(Size) \
  X(IsVectorData) \
  X(GetInt) \
  X(GetStr) \
  X(GetIntPos) \
  X(GetStrPos) \
  X(LvalInt) \
  X(LvalStr) \
  X(SetInt) \
  X(SetStr) \
  X(RemoveInt) \
  X(RemoveStr) \
  X(IterBegin) \
  X(IterLast) \
  X(IterEnd) \
  X(IterAdvance) \
  X(IterRewind) \
  X(Append) \
  X(Prepend) \
  X(Merge) \
  X(Pop) \
  X(Dequeue) \
  X(Renumber) \
  X(Copy) \
  X(ToVArray) \
  X(ToDArray) \
  X(ToVec) \
  X(ToDict) \
  X(ToKeyset)

enum class ArrayOp : uint8_t {
#define X(name) name,
ARRAY_OPS
#undef X
};

// We'll store a LoggingProfile for each array construction site SrcKey.
// It tracks the operations that happen on arrays coming from that site.
struct LoggingProfile {
  using EventMapKey = std::pair<SrcKey, std::string>;
  using EventMapHasher = pairHashCompare<
    SrcKey, std::string, SrcKey::TbbHashCompare, stringHashCompare>;

  // Values in the event map are sampled event counts.
  using EventMap = tbb::concurrent_hash_map<
    EventMapKey, size_t, EventMapHasher>;

  explicit LoggingProfile(SrcKey source) : source(source) {}

  size_t eventCount() const;

  // We take specific inputs rather than templated inputs because we're going
  // to follow up soon with limitations on the number of arguments we can log.
  void logEvent(ArrayOp op);
  void logEvent(ArrayOp op, int64_t k);
  void logEvent(ArrayOp op, const StringData* k);
  void logEvent(ArrayOp op, TypedValue v);
  void logEvent(ArrayOp op, int64_t k, TypedValue v);
  void logEvent(ArrayOp op, const StringData* k, TypedValue v);

private:
  void logEventImpl(const std::string& key);

public:
  SrcKey source;
  std::atomic<uint64_t> sampleCount = 0;
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
