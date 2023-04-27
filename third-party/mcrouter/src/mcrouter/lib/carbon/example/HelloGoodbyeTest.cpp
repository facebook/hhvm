/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>

#include "mcrouter/lib/carbon/example/gen/HelloGoodbyeRouterInfo.h"

TEST(HelloGoodbye, getTypeIdByName) {
  EXPECT_EQ(
      carbon::getTypeIdByName(
          "hello", hellogoodbye::HelloGoodbyeRouterInfo::RoutableRequests()),
      65);
  EXPECT_EQ(
      carbon::getTypeIdByName(
          "hello1", hellogoodbye::HelloGoodbyeRouterInfo::RoutableRequests()),
      0);
}
