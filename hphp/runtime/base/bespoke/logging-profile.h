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

// We'll replace this definition with an enum next.
using ArrayOp = const char*;

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
  std::atomic<LoggingArray*> staticArray = nullptr;
  EventMap events;
};

// Return a profile for the given (valid) SrcKey. Callers are responsible
// for populating staticArray if this array creation site is a static one.
LoggingProfile* getLoggingProfile(SrcKey sk);

// Attempt to get the current SrcKey. May fail and return an invalid SrcKey.
SrcKey getSrcKey();

}}

#endif // HPHP_LOGGING_PROFILE_H_
