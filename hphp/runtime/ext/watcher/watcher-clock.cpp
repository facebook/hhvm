/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source path is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the path LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/runtime/ext/watcher/watcher-clock.h"

namespace HPHP {
namespace Watcher {

// When we add in additional watchers, we whould ensure that the prefixes are distinct so a clock from one
// implementation does not get used with a different one.
const auto* kWatchmanClockSincePrefix = "since:";
const auto* kWatchmanMergebaseSincePrefix = "mergebase:";

WatcherClock WatcherClock::fromVariant(const Variant& clock) {
  if (clock.isString()) {
    auto clock_string = clock.getStringData()->toCppString();
    if (!clock_string.empty()) {
      std::string_view clock_view { clock_string };
      if (clock_view.starts_with(kWatchmanClockSincePrefix)) {
        clock_view.remove_prefix(strlen(kWatchmanClockSincePrefix));
        return { WatcherClockType::SINCE, std::string { clock_view } };
      }
      if (clock_view.starts_with(kWatchmanMergebaseSincePrefix)) {
        clock_view.remove_prefix(strlen(kWatchmanMergebaseSincePrefix));
        return { WatcherClockType::MERGEBASE, std::string { clock_view } };
      }
    }
  }
  return { WatcherClockType::NONE, std::nullopt };
}

WatcherClock WatcherClock::fromClock(WatcherClockType type, std::string value) {
  return { type, value };
}

std::string WatcherClock::toString() {
  if (!clock_value.has_value()) {
    return "";
  }

  switch (clock_type) {
    case WatcherClockType::SINCE:
      return std::string(kWatchmanClockSincePrefix) + clock_value.value();
    case WatcherClockType::MERGEBASE:
      return std::string(kWatchmanMergebaseSincePrefix) + clock_value.value();
    case WatcherClockType::NONE:
      return "";
  }
  return "";
}

}  // namespace Watcher
}  // namespace HPHP
