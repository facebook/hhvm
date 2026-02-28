/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <fmt/core.h>
#include "watchman/Logging.h"
#include "watchman/WatchmanConfig.h"

namespace watchman {

folly::Synchronized<std::string> poisoned_reason;

void set_poison_state(
    w_string_piece dir,
    std::chrono::system_clock::time_point now,
    const char* syscall,
    const std::error_code& err) {
  if (!poisoned_reason.rlock()->empty()) {
    return;
  }

  auto why = fmt::format(
      "A non-recoverable condition has triggered.  Watchman needs your help!\n"
      "The triggering condition was at timestamp={}: {}({}) -> {}\n"
      "All requests will continue to fail with this message until you resolve\n"
      "the underlying problem.  You will find more information on fixing this at\n"
      "{}#poison-{}\n",
      std::chrono::system_clock::to_time_t(now),
      syscall,
      dir.view(),
      err.message(),
      cfg_get_trouble_url(),
      syscall);

  watchman::log(watchman::ERR, why);
  *poisoned_reason.wlock() = why;
}

} // namespace watchman

/* vim:ts=2:sw=2:et:
 */
