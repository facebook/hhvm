/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/telemetry/WatchmanStructuredLogger.h"

#include <string>
#include <string_view>

#include <folly/portability/GTest.h>

#include "eden/common/telemetry/DynamicEvent.h"
#include "eden/common/telemetry/SessionInfo.h"
#include "eden/fs/telemetry/IXplatLogger.h"
#include "watchman/telemetry/LogEvent.h"
#include "watchman/telemetry/XplatKeys.h"

namespace {

using facebook::eden::DynamicEvent;
using facebook::eden::SessionInfo;

/// Captures the last event routed through the XplatLogger seam so the test can
/// assert the gate forwards fully-populated events under the right category.
class FakeXplatLogger : public facebook::eden::IXplatLogger {
 public:
  int callCount = 0;
  std::string lastCategory;
  DynamicEvent lastEvent;

  void logEvent(std::string_view category, const DynamicEvent& event) override {
    ++callCount;
    lastCategory = std::string{category};
    lastEvent = event;
  }
};

SessionInfo makeTestSessionInfo() {
  SessionInfo info;
  info.username = "alice";
  info.hostname = "devvm";
  info.os = "Linux";
  info.osVersion = "5.0";
  info.appVersion = "watchman-test";
  return info;
}

TEST(WatchmanXplatStructuredLoggerTest, routesEventToXplatLoggerWithIdentity) {
  auto fake = std::make_shared<FakeXplatLogger>();
  auto logger =
      watchman::makeWatchmanXplatStructuredLogger(makeTestSessionInfo(), fake);

  watchman::DispatchCommand event;
  event.root = "/data/users/alice/fbsource";
  event.command = "watch-project";
  logger->logEvent(event);

  EXPECT_EQ(1, fake->callCount);
  EXPECT_EQ(
      std::string{watchman::xplat_keys::kWatchmanEventsCategory},
      fake->lastCategory);

  const auto& strings = fake->lastEvent.getStringMap();
  // Identity fields, populated by
  // WatchmanStructuredLogger::populateDefaultFields.
  EXPECT_EQ("alice", strings.at("user"));
  EXPECT_EQ("devvm", strings.at("host"));
  EXPECT_EQ("watchman-test", strings.at("version"));
  EXPECT_EQ("watchman", strings.at("logged_by"));
  // Type comes from the TypedEvent overload.
  EXPECT_EQ("dispatch_command", strings.at("type"));
  // Event-specific fields.
  EXPECT_EQ("/data/users/alice/fbsource", strings.at("root"));
  EXPECT_EQ("watch-project", strings.at("command"));
}

} // namespace
