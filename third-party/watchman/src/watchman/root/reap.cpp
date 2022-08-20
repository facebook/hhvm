/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <fmt/chrono.h>
#include "watchman/Logging.h"
#include "watchman/root/Root.h"

using namespace watchman;

bool Root::considerReap() {
  if (idle_reap_age.count() == 0) {
    return false;
  }

  auto now = std::chrono::steady_clock::now();

  if (now > inner.last_cmd_timestamp.load(std::memory_order_acquire) +
              idle_reap_age &&
      (triggers.rlock()->empty()) && (now > inner.last_reap_timestamp) &&
      !unilateralResponses->hasSubscribers()) {
    // We haven't had any activity in a while, and there are no registered
    // triggers or subscriptions against this watch.
    log(ERR,
        "root ",
        root_path,
        " has had no activity in ",
        idle_reap_age,
        " and has no triggers or subscriptions, cancelling watch.  "
        "Set idle_reap_age_seconds in your .watchmanconfig to control this "
        "behavior\n");
    return true;
  }

  inner.last_reap_timestamp = now;

  return false;
}

/* vim:ts=2:sw=2:et:
 */
