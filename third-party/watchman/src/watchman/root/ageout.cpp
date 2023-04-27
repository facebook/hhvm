/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/QueryableView.h"
#include "watchman/root/Root.h"

using namespace watchman;

void Root::considerAgeOut() {
  if (gc_interval.count() == 0) {
    return;
  }

  auto now = std::chrono::system_clock::now();
  if (now <= view()->getLastAgeOutTimeStamp() + gc_interval) {
    // Don't check too often
    return;
  }

  performAgeOut(gc_age);
}

void Root::performAgeOut(std::chrono::seconds min_age) {
  // Find deleted nodes older than the gc_age setting.
  // This is particularly useful in cases where your tree observes a
  // large number of creates and deletes for many unique filenames in
  // a given dir (eg: temporary/randomized filenames generated as part
  // of build tooling or atomic renames)
  watchman::PerfSample sample("age_out");

  view()->ageOut(sample, std::chrono::seconds(min_age));

  // Age out cursors too.
  {
    auto cursors = inner.cursors.wlock();
    auto it = cursors->begin();
    while (it != cursors->end()) {
      if (it->second < view()->getLastAgeOutTickValue()) {
        it = cursors->erase(it);
      } else {
        ++it;
      }
    }
  }
  if (sample.finish()) {
    addPerfSampleMetadata(sample);
    sample.log();
  }
}

/* vim:ts=2:sw=2:et:
 */
