/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>

#include "folly/fibers/FiberManagerMap.h"
#include "folly/io/async/EventBase.h"
#include "folly/json/dynamic.h"
#include "folly/json/json.h"
#include "mcrouter/McrouterFiberContext.h"

using namespace ::testing;

namespace facebook {
namespace memcache {
namespace mcrouter {

namespace {
const int64_t kNetworkTransportTimeUs = 100;

struct RouterInfo {};

std::string runExtraDataCallbacks(
    const std::vector<ExtraDataCallbackT>& callbacks) {
  folly::dynamic json(folly::dynamic::object);
  for (const auto& cb : callbacks) {
    auto data = cb();
    for (const auto& [key, value] : data) {
      json[key] = value;
    }
  }
  folly::json::serialization_opts opts;
  opts.sort_keys = true;
  return folly::json::serialize(json, opts);
}
} // namespace

TEST(McrouterFiberContextTest, setAndGetNetworkTransportTimeUs) {
  folly::EventBase evb;
  auto& fm = folly::fibers::getFiberManagerT<fiber_local<RouterInfo>>(evb);
  fm.addTask([]() {
    EXPECT_EQ(fiber_local<RouterInfo>::getNetworkTransportTimeUs(), 0);
    fiber_local<RouterInfo>::incNetworkTransportTimeBy(kNetworkTransportTimeUs);
    EXPECT_EQ(
        fiber_local<RouterInfo>::getNetworkTransportTimeUs(),
        kNetworkTransportTimeUs);

    folly::fibers::addTask([] {
      EXPECT_EQ(
          fiber_local<RouterInfo>::getNetworkTransportTimeUs(),
          kNetworkTransportTimeUs);
      fiber_local<RouterInfo>::incNetworkTransportTimeBy(
          kNetworkTransportTimeUs);

      folly::fibers::addTask([]() {
        EXPECT_EQ(
            fiber_local<RouterInfo>::getNetworkTransportTimeUs(),
            2 * kNetworkTransportTimeUs);
      });
    });
  });
}

TEST(McrouterFiberContextTest, testExtraDataCallback) {
  folly::EventBase evb;
  auto& fm = folly::fibers::getFiberManagerT<fiber_local<RouterInfo>>(evb);
  fm.addTask([]() {
    fiber_local<RouterInfo>::addExtraDataCallbacks([]() -> ExtraDataMap {
      return {{"0", "0"}};
    });

    // Run in new fiber by copying fiber context, similar to shadow route.
    for (int i = 1; i < 6; i++) {
      auto ret = fiber_local<RouterInfo>::runWithLocals([&i]() {
        fiber_local<RouterInfo>::addExtraDataCallbacks([&i]() -> ExtraDataMap {
          return {{folly::to<std::string>(i), folly::to<std::string>(i)}};
        });
        return runExtraDataCallbacks(
            fiber_local<RouterInfo>::getExtraDataCallbacks());
      });
      EXPECT_EQ(ret, folly::sformat("{{\"0\":\"0\",\"{}\":\"{}\"}}", i, i));
    }

    // Run in new fiber without copying fiber context.
    for (int i = 1; i < 6; i++) {
      auto ret = fiber_local<RouterInfo>::runWithoutLocals([&i]() {
        fiber_local<RouterInfo>::addExtraDataCallbacks([&i]() -> ExtraDataMap {
          return {{folly::to<std::string>(i), folly::to<std::string>(i)}};
        });
        return runExtraDataCallbacks(
            fiber_local<RouterInfo>::getExtraDataCallbacks());
      });
      EXPECT_EQ(ret, folly::sformat("{{\"{}\":\"{}\"}}", i, i));
    }

    // Loop in the same fiber, similar to failover route. Keep replacing
    // callback function.
    int idx = -1;
    for (int i = 1; i < 6; i++) {
      if (idx == -1) {
        idx = static_cast<int>(fiber_local<RouterInfo>::addExtraDataCallbacks(
            [&i]() -> ExtraDataMap {
              return {{folly::to<std::string>(i), folly::to<std::string>(i)}};
            }));
      } else {
        fiber_local<RouterInfo>::updateExtraDataCallbacks(
            idx, [&i]() -> ExtraDataMap {
              return {{folly::to<std::string>(i), folly::to<std::string>(i)}};
            });
      }
      auto ret = runExtraDataCallbacks(
          fiber_local<RouterInfo>::getExtraDataCallbacks());
      EXPECT_EQ(ret, folly::sformat("{{\"0\":\"0\",\"{}\":\"{}\"}}", i, i));
    }
  });
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
