/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>

#include "mcrouter/CarbonRouterInstance.h"
#include "mcrouter/McrouterManager.h"
#include "mcrouter/lib/carbon/example/gen/HelloGoodbyeRouterInfo.h"
#include "mcrouter/options.h"
#include "mcrouter/routes/McrouterRouteHandle.h"

using namespace facebook::memcache;
using namespace facebook::memcache::mcrouter;

namespace {

McrouterOptions getTestOptions() {
  McrouterOptions opts = defaultTestOptions();
  opts.config = "{ \"route\": \"NullRoute\" }";
  return opts;
}

} // namespace

TEST(McrouterManagerTest, RouterInfoNameMatchesDynamicCast) {
  auto opts = getTestOptions();
  auto* router =
      CarbonRouterInstance<McrouterRouterInfo>::init("test_rinm_match", opts);
  ASSERT_NE(router, nullptr);

  CarbonRouterInstanceBase* base = router;

  auto* dynamicResult =
      dynamic_cast<CarbonRouterInstance<McrouterRouterInfo>*>(base);
  ASSERT_NE(dynamicResult, nullptr);

  EXPECT_EQ(base->routerInfoName(), McrouterRouterInfo::name);

  CarbonRouterInstance<McrouterRouterInfo>* staticResult = nullptr;
  if (base->routerInfoName() == McrouterRouterInfo::name) {
    staticResult = static_cast<CarbonRouterInstance<McrouterRouterInfo>*>(base);
  }
  EXPECT_EQ(staticResult, dynamicResult);
}

TEST(McrouterManagerTest, RouterInfoNameMismatchReturnNullLikeDynamicCast) {
  auto opts = getTestOptions();
  auto* router = CarbonRouterInstance<McrouterRouterInfo>::init(
      "test_rinm_mismatch", opts);
  ASSERT_NE(router, nullptr);

  CarbonRouterInstanceBase* base = router;

  auto* dynamicResult =
      dynamic_cast<CarbonRouterInstance<hellogoodbye::HelloGoodbyeRouterInfo>*>(
          base);
  EXPECT_EQ(dynamicResult, nullptr);

  EXPECT_NE(base->routerInfoName(), hellogoodbye::HelloGoodbyeRouterInfo::name);

  CarbonRouterInstance<hellogoodbye::HelloGoodbyeRouterInfo>* staticResult =
      nullptr;
  if (base->routerInfoName() == hellogoodbye::HelloGoodbyeRouterInfo::name) {
    staticResult = static_cast<
        CarbonRouterInstance<hellogoodbye::HelloGoodbyeRouterInfo>*>(base);
  }
  EXPECT_EQ(staticResult, dynamicResult);
}

TEST(McrouterManagerTest, NullBaseReturnNullLikeDynamicCast) {
  CarbonRouterInstanceBase* base = nullptr;

  auto* dynamicResult =
      dynamic_cast<CarbonRouterInstance<McrouterRouterInfo>*>(base);
  EXPECT_EQ(dynamicResult, nullptr);

  CarbonRouterInstance<McrouterRouterInfo>* staticResult = nullptr;
  if (base && base->routerInfoName() == McrouterRouterInfo::name) {
    staticResult = static_cast<CarbonRouterInstance<McrouterRouterInfo>*>(base);
  }
  EXPECT_EQ(staticResult, dynamicResult);
}
