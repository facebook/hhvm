/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/QueryableView.h"
#include "watchman/root/Root.h"
#include "watchman/telemetry/LogEvent.h"
#include "watchman/telemetry/WatchmanStructuredLogger.h"

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

  int64_t walked = 0;
  int64_t files = 0;
  int64_t dirs = 0;
  view()->ageOut(walked, files, dirs, std::chrono::seconds(min_age));

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
  auto root_metadata = getRootMetadata();

  if (sample.finish()) {
    sample.add_meta(
        "age_out",
        json_object(
            {{"walked", json_integer(walked)},
             {"files", json_integer(files)},
             {"dirs", json_integer(dirs)}}));

    sample.add_root_metadata(root_metadata);
    sample.log();
  }

  const auto& [samplingRate, eventCount] =
      getLogEventCounters(LogEventType::AgeOutType);
  // Log if override set, or if we have hit the sample rate
  if (sample.will_log || eventCount == samplingRate) {
    auto ageOut = AgeOut{
        // MetadataEvent
        {
            // BaseEvent
            {
                root_metadata.root_path.string(), // root
                std::string(), // error
                eventCount != samplingRate ? 0 : eventCount // event_count
            },
            root_metadata.recrawl_count, // recrawl
            root_metadata.case_sensitive, // case_sensitive
            root_metadata.watcher.string() // watcher
        },
        walked, // walked
        files, // files
        dirs // dirs
    };
    getLogger()->logEvent(ageOut);
  }
}

/* vim:ts=2:sw=2:et:
 */
