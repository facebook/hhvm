// Copyright 2004-present Facebook.  All rights reserved.
#pragma once

#include "folly/ThreadLocal.h"
#include <algorithm>
#include <chrono>
#include <cinttypes>

namespace facebook { namespace proxygen {

typedef std::chrono::system_clock::time_point TimePoint;

inline TimePoint getCurrentTime() {
  static folly::ThreadLocal<TimePoint> now_ms;
  *now_ms = std::max(std::chrono::system_clock::now(), *now_ms);
  return *now_ms;
}

inline std::chrono::milliseconds millisecondsSinceEpoch() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
    getCurrentTime().time_since_epoch());
}

inline std::chrono::seconds secondsSinceEpoch() {
  return std::chrono::duration_cast<std::chrono::seconds>(
    getCurrentTime().time_since_epoch());
}

inline std::chrono::milliseconds
millisecondsBetween(TimePoint finish, TimePoint start) {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
    finish - start);
}

inline std::chrono::seconds
secondsBetween(TimePoint finish, TimePoint start) {
  return std::chrono::duration_cast<std::chrono::seconds>(
    finish - start);
}

inline std::chrono::milliseconds millisecondsSince(TimePoint t) {
  return millisecondsBetween(getCurrentTime(), t);
}

inline std::chrono::milliseconds secondsSince(TimePoint t) {
  return secondsBetween(getCurrentTime(), t);
}

/**
 * Util class to be used to get real Time. We create a separate class
 * so that we can mock it the way we want.
 */
class TimeUtil {
 public:
  virtual ~TimeUtil() {}

  virtual TimePoint now() const {
    return getCurrentTime();
  }

  static const TimePoint& getZeroTimePoint() {
    const static TimePoint kZeroTimePoint{};
    return kZeroTimePoint;
  }
};

}}
