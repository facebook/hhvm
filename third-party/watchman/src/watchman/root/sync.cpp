/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "eden/common/utils/ProcessInfoCache.h"
#include "watchman/ClientContext.h"
#include "watchman/PerfSample.h"
#include "watchman/QueryableView.h"
#include "watchman/root/Root.h"
#include "watchman/telemetry/LogEvent.h"
#include "watchman/telemetry/WatchmanStructuredLogger.h"

using namespace watchman;

folly::SemiFuture<folly::Unit> Root::waitForSettle(
    std::chrono::milliseconds settle_period) {
  return view()->waitForSettle(settle_period);
}

CookieSync::SyncResult Root::syncToNow(
    std::chrono::milliseconds timeout,
    const ClientContext& client_info) {
  PerfSample sample("sync_to_now");
  SyncToNow syncToNow;
  auto root = shared_from_this();
  try {
    auto result = view()->syncToNow(root, timeout);
    auto root_metadata = getRootMetadata();

    if (sample.finish()) {
      sample.add_root_metadata(root_metadata);
      sample.add_meta(
          "sync_to_now",
          json_object(
              {{"success", json_boolean(true)},
               {"timeoutms", json_integer(timeout.count())}}));
      sample.log();
    }

    const auto& [samplingRate, eventCount] =
        getLogEventCounters(LogEventType::SyncToNowType);
    // Log if override set, or if we have hit the sample rate
    if (sample.will_log || eventCount == samplingRate) {
      syncToNow.client_pid = client_info.clientPid;
      syncToNow.client_name = client_info.clientInfo.has_value()
          ? facebook::eden::ProcessInfoCache::cleanProcessCommandline(
                std::move(client_info.clientInfo.value().get().name))
          : "";
      syncToNow.root = root_metadata.root_path.string();
      syncToNow.event_count = eventCount != samplingRate ? 0 : eventCount;
      syncToNow.recrawl = root_metadata.recrawl_count;
      syncToNow.case_sensitive = root_metadata.case_sensitive;
      syncToNow.watcher = root_metadata.watcher.string();
      syncToNow.timeoutms = timeout.count();
      getLogger()->logEvent(syncToNow);
    }
    return result;
  } catch (const std::exception& exc) {
    auto root_metadata = getRootMetadata();
    sample.force_log();
    sample.finish();
    sample.add_root_metadata(root_metadata);
    sample.add_meta(
        "sync_to_now",
        json_object(
            {{"success", json_boolean(false)},
             {"reason", w_string_to_json(exc.what())},
             {"timeoutms", json_integer(timeout.count())}}));
    sample.log();

    const auto& [samplingRate, eventCount] =
        getLogEventCounters(LogEventType::SyncToNowType);
    syncToNow.root = root_metadata.root_path.string();
    syncToNow.error = exc.what();
    syncToNow.event_count = eventCount != samplingRate ? 0 : eventCount;
    syncToNow.recrawl = root_metadata.recrawl_count;
    syncToNow.case_sensitive = root_metadata.case_sensitive;
    syncToNow.watcher = root_metadata.watcher.string();
    syncToNow.success = false;
    syncToNow.timeoutms = timeout.count();
    getLogger()->logEvent(syncToNow);

    throw;
  }
}

/* vim:ts=2:sw=2:et:
 */
