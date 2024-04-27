/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/telemetry/WatchmanStructuredLogger.h"

#include "eden/common/telemetry/StructuredLoggerFactory.h"
#include "watchman/WatchmanConfig.h"
#include "watchman/telemetry/WatchmanStats.h"

namespace watchman {

using UserInfo = facebook::eden::UserInfo;

WatchmanStructuredLogger::WatchmanStructuredLogger(
    std::shared_ptr<ScribeLogger> scribeLogger,
    SessionInfo sessionInfo)
    : ScubaStructuredLogger{std::move(scribeLogger), std::move(sessionInfo)} {}

std::shared_ptr<StructuredLogger> getLogger() {
  static std::shared_ptr<StructuredLogger> logger = facebook::eden::
      makeDefaultStructuredLogger<WatchmanStructuredLogger, WatchmanStatsPtr>(
          cfg_get_string("scribe-cat", ""),
          cfg_get_string("scribe-category", ""),
          facebook::eden::makeSessionInfo(
              UserInfo::lookup(),
              facebook::eden::getHostname(),
              PACKAGE_VERSION),
          getWatchmanStats());
  return logger;
}

DynamicEvent WatchmanStructuredLogger::populateDefaultFields(const char* type) {
  DynamicEvent event = StructuredLogger::populateDefaultFields(type);
  if (sessionInfo_.ciInstanceId.has_value()) {
    event.addInt("sandcastle_instance_id", *sessionInfo_.ciInstanceId);
  }
  event.addString("version", sessionInfo_.appVersion);
#ifdef WATCHMAN_BUILD_INFO
  event.addString("buildinfo", WATCHMAN_BUILD_INFO);
#endif
  event.addString("logged_by", "watchman");
  return event;
}

} // namespace watchman
