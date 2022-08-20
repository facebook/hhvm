/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <memory>
#include <random>
#include <vector>

#include <gtest/gtest.h>

#include "mcrouter/lib/network/gen/MemcacheMessages.h"
#include "mcrouter/routes/OutstandingLimitRoute.h"
#include "mcrouter/routes/test/RouteHandleTestUtil.h"

using namespace facebook::memcache;
using namespace facebook::memcache::mcrouter;

namespace {

std::string makeKey(uint64_t id) {
  return folly::sformat("test-key:{}", id);
}

} // anonymous namespace

void sendRequest(
    folly::fibers::FiberManager& fm,
    McrouterRouteHandleIf& rh,
    size_t id,
    uint64_t senderId,
    std::vector<std::string>& replyOrder) {
  auto context = getTestContext();
  context->setSenderIdForTest(senderId);

  fm.addTask([&rh, id, context, &replyOrder]() {
    McGetRequest request(makeKey(id));
    fiber_local<MemcacheRouterInfo>::setSharedCtx(std::move(context));
    rh.route(request);
    replyOrder.push_back(makeKey(id));
  });
}

TEST(oustandingLimitRouteTest, basic) {
  auto normalHandle = std::make_shared<TestHandle>(
      GetRouteTestData(carbon::Result::FOUND, "a"));

  McrouterRouteHandle<OutstandingLimitRoute<McrouterRouterInfo>> rh(
      normalHandle->rh, 3);

  normalHandle->pause();

  std::vector<std::string> replyOrder;

  TestFiberManager testfm{
      typename fiber_local<MemcacheRouterInfo>::ContextTypeTag()};
  auto& fm = testfm.getFiberManager();

  sendRequest(fm, rh, 1, 1, replyOrder);
  sendRequest(fm, rh, 2, 1, replyOrder);
  sendRequest(fm, rh, 3, 1, replyOrder);
  sendRequest(fm, rh, 4, 1, replyOrder);
  sendRequest(fm, rh, 5, 2, replyOrder);
  sendRequest(fm, rh, 6, 2, replyOrder);
  sendRequest(fm, rh, 7, 2, replyOrder);
  sendRequest(fm, rh, 8, 1, replyOrder);
  sendRequest(fm, rh, 9, 0, replyOrder);
  sendRequest(fm, rh, 10, 3, replyOrder);
  sendRequest(fm, rh, 11, 0, replyOrder);
  sendRequest(fm, rh, 12, 4, replyOrder);
  sendRequest(fm, rh, 13, 3, replyOrder);
  sendRequest(fm, rh, 14, 0, replyOrder);

  auto& loopController =
      dynamic_cast<folly::fibers::SimpleLoopController&>(fm.loopController());
  loopController.loop([&]() {
    fm.addTask([&]() { normalHandle->unpause(); });
    loopController.stop();
  });

  EXPECT_EQ(14, replyOrder.size());
  EXPECT_EQ(makeKey(1), replyOrder[0]);
  EXPECT_EQ(makeKey(2), replyOrder[1]);
  EXPECT_EQ(makeKey(3), replyOrder[2]);
  EXPECT_EQ(makeKey(4), replyOrder[3]);
  EXPECT_EQ(makeKey(5), replyOrder[4]);
  EXPECT_EQ(makeKey(9), replyOrder[5]);
  EXPECT_EQ(makeKey(10), replyOrder[6]);
  EXPECT_EQ(makeKey(11), replyOrder[7]);
  EXPECT_EQ(makeKey(12), replyOrder[8]);
  EXPECT_EQ(makeKey(14), replyOrder[9]);
  EXPECT_EQ(makeKey(8), replyOrder[10]);
  EXPECT_EQ(makeKey(6), replyOrder[11]);
  EXPECT_EQ(makeKey(13), replyOrder[12]);
  EXPECT_EQ(makeKey(7), replyOrder[13]);
}
