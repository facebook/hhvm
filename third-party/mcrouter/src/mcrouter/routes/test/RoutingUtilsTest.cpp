/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/Range.h>
#include <folly/json/dynamic.h>
#include <gtest/gtest.h>

#include "mcrouter/routes/RoutingUtils.h"

namespace facebook::memcache::mcrouter {

namespace {
constexpr folly::StringPiece kServer = "localhost:5000";
constexpr folly::StringPiece kTwJob = "tsp_usm1/fci/aggregator";

// Wraps a single service entry into a `services` object keyed by kServer.
folly::dynamic servicesWith(folly::dynamic service) {
  return folly::dynamic::object(kServer, std::move(service));
}

folly::dynamic serviceWithTwJob(folly::dynamic twJob) {
  return folly::dynamic::object(
      "props", folly::dynamic::object("tw_job", std::move(twJob)));
}
} // namespace

TEST(GetTwJobFromServices, nullServicesReturnsNullopt) {
  EXPECT_FALSE(getTwJobFromServices(nullptr, kServer).has_value());
}

TEST(GetTwJobFromServices, nonObjectServicesReturnsNullopt) {
  auto services = folly::dynamic::array(1, 2, 3);
  EXPECT_FALSE(getTwJobFromServices(&services, kServer).has_value());
}

TEST(GetTwJobFromServices, missingServerReturnsNullopt) {
  auto services = servicesWith(serviceWithTwJob(kTwJob));
  EXPECT_FALSE(getTwJobFromServices(&services, "localhost:9999").has_value());
}

TEST(GetTwJobFromServices, scalarServiceReturnsNullopt) {
  auto services = servicesWith("not-an-object");
  EXPECT_FALSE(getTwJobFromServices(&services, kServer).has_value());
}

TEST(GetTwJobFromServices, missingPropsReturnsNullopt) {
  auto services = servicesWith(folly::dynamic::object("other", 1));
  EXPECT_FALSE(getTwJobFromServices(&services, kServer).has_value());
}

TEST(GetTwJobFromServices, scalarPropsReturnsNullopt) {
  auto services = servicesWith(folly::dynamic::object("props", "scalar"));
  EXPECT_FALSE(getTwJobFromServices(&services, kServer).has_value());
}

TEST(GetTwJobFromServices, missingTwJobReturnsNullopt) {
  auto services =
      servicesWith(folly::dynamic::object("props", folly::dynamic::object()));
  EXPECT_FALSE(getTwJobFromServices(&services, kServer).has_value());
}

TEST(GetTwJobFromServices, nonStringTwJobReturnsNullopt) {
  auto services = servicesWith(serviceWithTwJob(42));
  EXPECT_FALSE(getTwJobFromServices(&services, kServer).has_value());
}

TEST(GetTwJobFromServices, emptyTwJobReturnsNullopt) {
  auto services = servicesWith(serviceWithTwJob(""));
  EXPECT_FALSE(getTwJobFromServices(&services, kServer).has_value());
}

TEST(GetTwJobFromServices, validTwJobReturnsValue) {
  auto services = servicesWith(serviceWithTwJob(kTwJob));
  auto twJob = getTwJobFromServices(&services, kServer);
  ASSERT_TRUE(twJob.has_value());
  EXPECT_EQ(kTwJob, *twJob);
}

} // namespace facebook::memcache::mcrouter
