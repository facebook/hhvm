/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <memory>
#include <unordered_map>
#include <vector>

#include <folly/fibers/FiberManager.h>
#include <gtest/gtest.h>

#include "mcrouter/CarbonRouterInstanceBase.h"
#include "mcrouter/McrouterFiberContext.h"
#include "mcrouter/lib/WeightedCh3HashFunc.h"
#include "mcrouter/lib/carbon/RequestReplyUtil.h"
#include "mcrouter/lib/mc/msg.h"
#include "mcrouter/lib/network/MessageHelpers.h"
#include "mcrouter/lib/network/ServerLoad.h"
#include "mcrouter/lib/network/gen/MemcacheRouterInfo.h"
#include "mcrouter/lib/test/TestRouteHandle.h"
#include "mcrouter/routes/HashRouteFactory.h"
#include "mcrouter/routes/LoadBalancerRoute.h"

using namespace facebook::memcache;
using namespace facebook::memcache::mcrouter;

namespace {

template <class Reply>
typename std::enable_if<HasValueTrait<Reply>::value, void>::type setReplyValue(
    Reply& reply,
    std::unordered_map<std::string, double>& mymap,
    const std::string& val) {
  auto it = mymap.find(val);

  if (it != mymap.end()) {
    mcrouter::fiber_local<TestRouterInfo>::setServerLoad(
        ServerLoad(ServerLoad::fromPercentLoad(it->second)));
  }

  reply.value_ref() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, val);
}
template <class Reply>
typename std::enable_if<!HasValueTrait<Reply>::value, void>::type setReplyValue(
    Reply&,
    std::unordered_map<std::string, double>&,
    const std::string&) {}

template <class RouteHandleIf>
class TestRoute {
 public:
  explicit TestRoute(
      std::string name,
      std::unordered_map<std::string, double> mymap,
      carbon::Result result = carbon::Result::OK)
      : name_(std::move(name)), map_(std::move(mymap)), result_(result) {}

  template <class Request>
  bool traverse(const Request&, const RouteHandleTraverser<RouteHandleIf>&)
      const {
    return false;
  }

  template <class Request>
  ReplyT<Request> route(const Request& /* req */) {
    ReplyT<Request> reply(result_);
    if (carbon::GetLike<Request>::value) {
      setReplyValue(reply, map_, name_);
    }
    return reply;
  }

  static std::string routeName() {
    return "test-route";
  }

 private:
  std::string name_;
  std::unordered_map<std::string, double> map_;
  carbon::Result result_;
};

} // anonymous namespace

TEST(LoadBalancerRouteTest, basic) {
  std::unordered_map<std::string, double> mymap;
  mymap.emplace("cpua", 25);
  mymap.emplace("cpub", 50);
  mymap.emplace("cpuc", 75);
  std::vector<std::shared_ptr<TestRouteHandleIf>> testHandles{
      makeRouteHandle<TestRouteHandleIf, TestRoute>("cpua", mymap),
      makeRouteHandle<TestRouteHandleIf, TestRoute>("cpub", mymap),
      makeRouteHandle<TestRouteHandleIf, TestRoute>("cpuc", mymap)};

  TestRouteHandle<LoadBalancerRoute<TestRouterInfo>> rh(
      testHandles,
      "",
      std::chrono::milliseconds(100),
      /* failoverCount */ 1);

  std::unordered_map<std::string, size_t> cmap;
  for (int i = 0; i < 100; i++) {
    auto reply = rh.route(McGetRequest("0" + std::to_string(i)));
    std::string v = carbon::valueRangeSlow(reply).str();
    auto it = cmap.find(v);
    if (it != cmap.end()) {
      cmap[std::string(v)]++;
    } else {
      cmap.emplace(std::string(v), 1);
    }
  }
  LOG(INFO) << cmap["cpua"] << " " << cmap["cpub"] << " " << cmap["cpuc"];
  EXPECT_TRUE((cmap["cpua"] >= 56) && (cmap["cpua"] <= 65));
  EXPECT_TRUE((cmap["cpub"] >= 25) && (cmap["cpub"] <= 30));
  EXPECT_TRUE((cmap["cpuc"] >= 10) && (cmap["cpuc"] <= 15));
}

TEST(LoadBalancerRouteTest, oneFullyLoaded) {
  std::unordered_map<std::string, double> mymap;
  mymap.emplace("cpua", 50);
  mymap.emplace("cpub", 50);
  mymap.emplace("cpuc", 100);
  std::vector<std::shared_ptr<TestRouteHandleIf>> testHandles{
      makeRouteHandle<TestRouteHandleIf, TestRoute>("cpua", mymap),
      makeRouteHandle<TestRouteHandleIf, TestRoute>("cpub", mymap),
      makeRouteHandle<TestRouteHandleIf, TestRoute>("cpuc", mymap)};

  TestRouteHandle<LoadBalancerRoute<TestRouterInfo>> rh(
      testHandles,
      "SALT-STRING",
      std::chrono::milliseconds(100),
      /* failoverCount */ 1);

  std::unordered_map<std::string, size_t> cmap;
  for (int i = 0; i < 100; i++) {
    auto reply = rh.route(McGetRequest("0" + std::to_string(i)));
    std::string v = carbon::valueRangeSlow(reply).str();
    auto it = cmap.find(v);
    if (it != cmap.end()) {
      cmap[std::string(v)]++;
    } else {
      cmap.emplace(std::string(v), 1);
    }
  }
  LOG(INFO) << cmap["cpua"] << " " << cmap["cpub"] << " " << cmap["cpuc"];
  EXPECT_TRUE((cmap["cpua"] >= 40) && (cmap["cpua"] <= 60));
  EXPECT_TRUE((cmap["cpub"] >= 40) && (cmap["cpub"] <= 60));
  EXPECT_TRUE((cmap["cpuc"] >= 0) && (cmap["cpuc"] <= 1));
}

TEST(LoadBalancerRouteTest, oneZeroLoad) {
  std::unordered_map<std::string, double> mymap;
  mymap.emplace("cpua", 0);
  mymap.emplace("cpub", 50);
  mymap.emplace("cpuc", 50);
  std::vector<std::shared_ptr<TestRouteHandleIf>> testHandles{
      makeRouteHandle<TestRouteHandleIf, TestRoute>("cpua", mymap),
      makeRouteHandle<TestRouteHandleIf, TestRoute>("cpub", mymap),
      makeRouteHandle<TestRouteHandleIf, TestRoute>("cpuc", mymap)};

  TestRouteHandle<LoadBalancerRoute<TestRouterInfo>> rh(
      testHandles,
      "TEST-SALT",
      std::chrono::milliseconds(100),
      /* failoverCount */ 1);

  std::unordered_map<std::string, size_t> cmap;
  for (int i = 0; i < 100; i++) {
    auto reply = rh.route(McGetRequest("0" + std::to_string(i)));
    std::string v = carbon::valueRangeSlow(reply).str();
    auto it = cmap.find(v);
    if (it != cmap.end()) {
      cmap[std::string(v)]++;
    } else {
      cmap.emplace(std::string(v), 1);
    }
  }
  LOG(INFO) << cmap["cpua"] << " " << cmap["cpub"] << " " << cmap["cpuc"];
  EXPECT_TRUE((cmap["cpua"] >= 40) && (cmap["cpua"] <= 60));
  EXPECT_TRUE((cmap["cpub"] >= 20) && (cmap["cpub"] <= 35));
  EXPECT_TRUE((cmap["cpuc"] >= 20) && (cmap["cpuc"] <= 35));
}

TEST(LoadBalancerRouteTest, AllFullyLoaded) {
  std::unordered_map<std::string, double> mymap;
  mymap.emplace("cpua", 100);
  mymap.emplace("cpub", 100);
  mymap.emplace("cpuc", 100);
  std::vector<std::shared_ptr<TestRouteHandleIf>> testHandles{
      makeRouteHandle<TestRouteHandleIf, TestRoute>("cpua", mymap),
      makeRouteHandle<TestRouteHandleIf, TestRoute>("cpub", mymap),
      makeRouteHandle<TestRouteHandleIf, TestRoute>("cpuc", mymap)};

  TestRouteHandle<LoadBalancerRoute<TestRouterInfo>> rh(
      testHandles,
      "TEST-SALT",
      std::chrono::milliseconds(100),
      /* failoverCount */ 1);

  std::unordered_map<std::string, size_t> cmap;
  for (int i = 0; i < 100; i++) {
    auto reply = rh.route(McGetRequest("0" + std::to_string(i)));
    std::string v = carbon::valueRangeSlow(reply).str();
    auto it = cmap.find(v);
    if (it != cmap.end()) {
      cmap[std::string(v)]++;
    } else {
      cmap.emplace(std::string(v), 1);
    }
  }
  LOG(INFO) << cmap["cpua"] << " " << cmap["cpub"] << " " << cmap["cpuc"];
  EXPECT_TRUE((cmap["cpua"] >= 25) && (cmap["cpua"] <= 45));
  EXPECT_TRUE((cmap["cpub"] >= 25) && (cmap["cpub"] <= 45));
  EXPECT_TRUE((cmap["cpuc"] >= 25) && (cmap["cpuc"] <= 45));
}

TEST(LoadBalancerRouteTest, AllZeroLoads) {
  std::unordered_map<std::string, double> mymap;
  mymap.emplace("cpua", 0);
  mymap.emplace("cpub", 0);
  mymap.emplace("cpuc", 0);
  std::vector<std::shared_ptr<TestRouteHandleIf>> testHandles{
      makeRouteHandle<TestRouteHandleIf, TestRoute>("cpua", mymap),
      makeRouteHandle<TestRouteHandleIf, TestRoute>("cpub", mymap),
      makeRouteHandle<TestRouteHandleIf, TestRoute>("cpuc", mymap)};

  TestRouteHandle<LoadBalancerRoute<TestRouterInfo>> rh(
      testHandles,
      "TEST-SALT",
      std::chrono::milliseconds(100),
      /* failoverCount */ 1);

  std::unordered_map<std::string, size_t> cmap;
  for (int i = 0; i < 100; i++) {
    auto reply = rh.route(McGetRequest("0" + std::to_string(i)));
    std::string v = carbon::valueRangeSlow(reply).str();
    auto it = cmap.find(v);
    if (it != cmap.end()) {
      cmap[std::string(v)]++;
    } else {
      cmap.emplace(std::string(v), 1);
    }
  }
  LOG(INFO) << cmap["cpua"] << " " << cmap["cpub"] << " " << cmap["cpuc"];
  EXPECT_TRUE((cmap["cpua"] >= 25) && (cmap["cpua"] <= 45));
  EXPECT_TRUE((cmap["cpub"] >= 25) && (cmap["cpub"] <= 45));
  EXPECT_TRUE((cmap["cpuc"] >= 25) && (cmap["cpuc"] <= 45));
}

TEST(LoadBalancerRouteTest, LoadsWithWait) {
  std::unordered_map<std::string, double> mymap;
  mymap.emplace("cpua", 100);
  mymap.emplace("cpub", 50);
  mymap.emplace("cpuc", 50);
  std::vector<std::shared_ptr<TestRouteHandleIf>> testHandles{
      makeRouteHandle<TestRouteHandleIf, TestRoute>("cpua", mymap),
      makeRouteHandle<TestRouteHandleIf, TestRoute>("cpub", mymap),
      makeRouteHandle<TestRouteHandleIf, TestRoute>("cpuc", mymap)};

  TestRouteHandle<LoadBalancerRoute<TestRouterInfo>> rh(
      testHandles,
      "TEST-SALT",
      std::chrono::milliseconds(100),
      /* failoverCount */ 1);

  std::unordered_map<std::string, size_t> cmap;
  for (int i = 0; i < 100; i++) {
    auto reply = rh.route(McGetRequest("0" + std::to_string(i)));
    std::string v = carbon::valueRangeSlow(reply).str();
    auto it = cmap.find(v);
    if (it != cmap.end()) {
      cmap[std::string(v)]++;
    } else {
      cmap.emplace(std::string(v), 1);
    }
    // sleep here to cause server load of 'cpua' to expire
    if (i > 25) {
      /* sleep override */
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }
  LOG(INFO) << cmap["cpua"] << " " << cmap["cpub"] << " " << cmap["cpuc"];
  EXPECT_TRUE((cmap["cpua"] >= 10) && (cmap["cpua"] <= 20));
  EXPECT_TRUE((cmap["cpub"] >= 25) && (cmap["cpub"] <= 45));
  EXPECT_TRUE((cmap["cpuc"] >= 25) && (cmap["cpuc"] <= 45));
}

TEST(LoadBalancerRouteTest, failover) {
  std::unordered_map<std::string, double> mymap;
  mymap.emplace("cpua", 30);
  mymap.emplace("cpub", 50);
  mymap.emplace("cpuc", 70);
  std::vector<std::shared_ptr<TestRouteHandleIf>> testHandles{
      makeRouteHandle<TestRouteHandleIf, TestRoute>(
          "cpua", mymap, carbon::Result::TIMEOUT),
      makeRouteHandle<TestRouteHandleIf, TestRoute>(
          "cpub", mymap, carbon::Result::REMOTE_ERROR),
      makeRouteHandle<TestRouteHandleIf, TestRoute>(
          "cpuc", mymap, carbon::Result::OK)};

  TestRouteHandle<LoadBalancerRoute<TestRouterInfo>> rh1Failover{
      testHandles,
      "",
      std::chrono::milliseconds(100),
      /* failoverCount */ 1};
  TestRouteHandle<LoadBalancerRoute<TestRouterInfo>> rh2Failover{
      testHandles,
      "",
      std::chrono::milliseconds(100),
      /* failoverCount */ 2};
  TestRouteHandle<LoadBalancerRoute<TestRouterInfo>> rh3Failover{
      testHandles,
      "",
      std::chrono::milliseconds(100),
      /* failoverCount */ 3};

  // warm-up the route handles (so that all children have their final server
  // load).
  for (size_t i = 0; i < 100; ++i) {
    rh1Failover.route(McGetRequest(folly::to<std::string>(i)));
    rh2Failover.route(McGetRequest(folly::to<std::string>(i)));
    rh3Failover.route(McGetRequest(folly::to<std::string>(i)));
  }

  // success on the first try.
  std::string key = "req08";
  auto reply1 = rh1Failover.route(McGetRequest(key));
  auto reply2 = rh2Failover.route(McGetRequest(key));
  auto reply3 = rh3Failover.route(McGetRequest(key));
  EXPECT_EQ(carbon::Result::OK, *reply1.result_ref());
  EXPECT_EQ(carbon::Result::OK, *reply2.result_ref());
  EXPECT_EQ(carbon::Result::OK, *reply3.result_ref());

  // success on the second try.
  key = "req05";
  reply1 = rh1Failover.route(McGetRequest(key));
  reply2 = rh2Failover.route(McGetRequest(key));
  reply3 = rh3Failover.route(McGetRequest(key));
  EXPECT_EQ(carbon::Result::REMOTE_ERROR, *reply1.result_ref());
  EXPECT_EQ(carbon::Result::OK, *reply2.result_ref());
  EXPECT_EQ(carbon::Result::OK, *reply3.result_ref());

  // success on the third try.
  key = "req10";
  reply1 = rh1Failover.route(McGetRequest(key));
  reply2 = rh2Failover.route(McGetRequest(key));
  reply3 = rh3Failover.route(McGetRequest(key));
  EXPECT_EQ(carbon::Result::REMOTE_ERROR, *reply1.result_ref());
  EXPECT_EQ(carbon::Result::TIMEOUT, *reply2.result_ref());
  EXPECT_EQ(carbon::Result::OK, *reply3.result_ref());
}

TEST(LoadBalancerRouteTest, failoverStress) {
  std::unordered_map<std::string, double> mymap;
  mymap.emplace("cpua", 30);
  mymap.emplace("cpub", 50);
  mymap.emplace("cpuc", 70);
  std::vector<std::shared_ptr<TestRouteHandleIf>> testHandles{
      makeRouteHandle<TestRouteHandleIf, TestRoute>(
          "cpua", mymap, carbon::Result::TIMEOUT),
      makeRouteHandle<TestRouteHandleIf, TestRoute>(
          "cpub", mymap, carbon::Result::REMOTE_ERROR),
      makeRouteHandle<TestRouteHandleIf, TestRoute>(
          "cpuc", mymap, carbon::Result::OK)};

  TestRouteHandle<LoadBalancerRoute<TestRouterInfo>> rh{
      testHandles,
      "",
      std::chrono::milliseconds(100),
      /* failoverCount */ 3};

  for (size_t i = 0; i < 1000; ++i) {
    auto reply = rh.route(McGetRequest(folly::to<std::string>(i)));
    EXPECT_EQ(carbon::Result::OK, *reply.result_ref());
    EXPECT_EQ("cpuc", carbon::valueRangeSlow(reply).str());
  }
}

TEST(CpuLoadBalancerRouteTest, basicTwoChoice) {
  std::unordered_map<std::string, double> mymap;
  mymap.emplace("cpua", 25);
  mymap.emplace("cpub", 50);
  mymap.emplace("cpuc", 75);
  std::vector<std::shared_ptr<TestRouteHandleIf>> testHandles{
      makeRouteHandle<TestRouteHandleIf, TestRoute>("cpua", mymap),
      makeRouteHandle<TestRouteHandleIf, TestRoute>("cpub", mymap),
      makeRouteHandle<TestRouteHandleIf, TestRoute>("cpuc", mymap)};

  TestRouteHandle<LoadBalancerRoute<TestRouterInfo>> rh(
      testHandles,
      "",
      std::chrono::milliseconds(100),
      /* fauloverCount */ 1,
      LoadBalancerRoute<TestRouterInfo>::AlgorithmType::TWO_RANDOM_CHOICES,
      /* fixed seed */ 0);

  std::unordered_map<std::string, size_t> cmap;
  for (int i = 0; i < 100; i++) {
    auto reply = rh.route(McGetRequest("0" + std::to_string(i)));
    std::string v = carbon::valueRangeSlow(reply).str();
    auto it = cmap.find(v);
    if (it != cmap.end()) {
      cmap[std::string(v)]++;
    } else {
      cmap.emplace(std::string(v), 1);
    }
  }
  LOG(INFO) << cmap["cpua"] << " " << cmap["cpub"] << " " << cmap["cpuc"];
  EXPECT_TRUE((cmap["cpua"] >= 60) && (cmap["cpua"] <= 72));
  EXPECT_TRUE((cmap["cpub"] >= 27) && (cmap["cpub"] <= 39));
  EXPECT_TRUE((cmap["cpuc"] >= 0) && (cmap["cpuc"] <= 1));
}

TEST(CpuLoadBalancerRouteTest, basicTwoChoiceWithSeed) {
  std::unordered_map<std::string, double> mymap;
  mymap.emplace("cpua", 25);
  mymap.emplace("cpub", 50);
  mymap.emplace("cpuc", 75);
  std::vector<std::shared_ptr<TestRouteHandleIf>> testHandles{
      makeRouteHandle<TestRouteHandleIf, TestRoute>("cpua", mymap),
      makeRouteHandle<TestRouteHandleIf, TestRoute>("cpub", mymap),
      makeRouteHandle<TestRouteHandleIf, TestRoute>("cpuc", mymap)};

  TestRouteHandle<LoadBalancerRoute<TestRouterInfo>> rh(
      testHandles,
      "",
      std::chrono::milliseconds(100),
      /* fauloverCount */ 1,
      LoadBalancerRoute<TestRouterInfo>::AlgorithmType::TWO_RANDOM_CHOICES,
      /* fixed seed */ 0);

  std::unordered_map<std::string, size_t> cmap;
  for (int i = 0; i < 100; i++) {
    auto reply = rh.route(McGetRequest("0" + std::to_string(i)));
    std::string v = carbon::valueRangeSlow(reply).str();
    auto it = cmap.find(v);
    if (it != cmap.end()) {
      cmap[std::string(v)]++;
    } else {
      cmap.emplace(std::string(v), 1);
    }
  }
  LOG(INFO) << cmap["cpua"] << " " << cmap["cpub"] << " " << cmap["cpuc"];
  EXPECT_TRUE((cmap["cpua"] >= 60) && (cmap["cpua"] <= 72));
  EXPECT_TRUE((cmap["cpub"] >= 27) && (cmap["cpub"] <= 39));
  EXPECT_TRUE((cmap["cpuc"] >= 0) && (cmap["cpuc"] <= 1));
}

TEST(CpuLoadBalancerRouteTest, oneFullyLoadedTwoChoice) {
  std::unordered_map<std::string, double> mymap;
  mymap.emplace("cpua", 50);
  mymap.emplace("cpub", 50);
  mymap.emplace("cpuc", 100);
  std::vector<std::shared_ptr<TestRouteHandleIf>> testHandles{
      makeRouteHandle<TestRouteHandleIf, TestRoute>("cpua", mymap),
      makeRouteHandle<TestRouteHandleIf, TestRoute>("cpub", mymap),
      makeRouteHandle<TestRouteHandleIf, TestRoute>("cpuc", mymap)};

  TestRouteHandle<LoadBalancerRoute<TestRouterInfo>> rh(
      testHandles,
      "SALT-STRING",
      std::chrono::milliseconds(100),
      /* fauloverCount */ 1,
      LoadBalancerRoute<TestRouterInfo>::AlgorithmType::TWO_RANDOM_CHOICES,
      /* fixed seed */ 0);

  std::unordered_map<std::string, size_t> cmap;
  for (int i = 0; i < 100; i++) {
    auto reply = rh.route(McGetRequest("0" + std::to_string(i)));
    std::string v = carbon::valueRangeSlow(reply).str();
    auto it = cmap.find(v);
    if (it != cmap.end()) {
      cmap[std::string(v)]++;
    } else {
      cmap.emplace(std::string(v), 1);
    }
  }
  LOG(INFO) << cmap["cpua"] << " " << cmap["cpub"] << " " << cmap["cpuc"];
  EXPECT_TRUE((cmap["cpua"] >= 40) && (cmap["cpua"] <= 60));
  EXPECT_TRUE((cmap["cpub"] >= 40) && (cmap["cpub"] <= 60));
  EXPECT_TRUE((cmap["cpuc"] >= 0) && (cmap["cpuc"] <= 1));
}

TEST(CpuLoadBalancerRouteTest, oneZeroLoadTwoChoice) {
  std::unordered_map<std::string, double> mymap;
  mymap.emplace("cpua", 0);
  mymap.emplace("cpub", 50);
  mymap.emplace("cpuc", 50);
  std::vector<std::shared_ptr<TestRouteHandleIf>> testHandles{
      makeRouteHandle<TestRouteHandleIf, TestRoute>("cpua", mymap),
      makeRouteHandle<TestRouteHandleIf, TestRoute>("cpub", mymap),
      makeRouteHandle<TestRouteHandleIf, TestRoute>("cpuc", mymap)};

  TestRouteHandle<LoadBalancerRoute<TestRouterInfo>> rh(
      testHandles,
      "TEST-SALT",
      std::chrono::milliseconds(100),
      /* fauloverCount */ 1,
      LoadBalancerRoute<TestRouterInfo>::AlgorithmType::TWO_RANDOM_CHOICES,
      /* fixed seed */ 0);

  std::unordered_map<std::string, size_t> cmap;
  for (int i = 0; i < 100; i++) {
    auto reply = rh.route(McGetRequest("0" + std::to_string(i)));
    std::string v = carbon::valueRangeSlow(reply).str();
    auto it = cmap.find(v);
    if (it != cmap.end()) {
      cmap[std::string(v)]++;
    } else {
      cmap.emplace(std::string(v), 1);
    }
  }
  LOG(INFO) << cmap["cpua"] << " " << cmap["cpub"] << " " << cmap["cpuc"];
  EXPECT_TRUE((cmap["cpua"] >= 55) && (cmap["cpua"] <= 75));
  EXPECT_TRUE((cmap["cpub"] >= 10) && (cmap["cpub"] <= 25));
  EXPECT_TRUE((cmap["cpuc"] >= 10) && (cmap["cpuc"] <= 25));
}

TEST(CpuLoadBalancerRouteTest, AllFullyLoadedTwoChoice) {
  std::unordered_map<std::string, double> mymap;
  mymap.emplace("cpua", 100);
  mymap.emplace("cpub", 100);
  mymap.emplace("cpuc", 100);
  std::vector<std::shared_ptr<TestRouteHandleIf>> testHandles{
      makeRouteHandle<TestRouteHandleIf, TestRoute>("cpua", mymap),
      makeRouteHandle<TestRouteHandleIf, TestRoute>("cpub", mymap),
      makeRouteHandle<TestRouteHandleIf, TestRoute>("cpuc", mymap)};

  TestRouteHandle<LoadBalancerRoute<TestRouterInfo>> rh(
      testHandles,
      "TEST-SALT",
      std::chrono::milliseconds(100),
      /* fauloverCount */ 1,
      LoadBalancerRoute<TestRouterInfo>::AlgorithmType::TWO_RANDOM_CHOICES,
      /* fixed seed */ 0);

  std::unordered_map<std::string, size_t> cmap;
  for (int i = 0; i < 100; i++) {
    auto reply = rh.route(McGetRequest("0" + std::to_string(i)));
    std::string v = carbon::valueRangeSlow(reply).str();
    auto it = cmap.find(v);
    if (it != cmap.end()) {
      cmap[std::string(v)]++;
    } else {
      cmap.emplace(std::string(v), 1);
    }
  }
  LOG(INFO) << cmap["cpua"] << " " << cmap["cpub"] << " " << cmap["cpuc"];
  EXPECT_TRUE((cmap["cpua"] >= 25) && (cmap["cpua"] <= 45));
  EXPECT_TRUE((cmap["cpub"] >= 25) && (cmap["cpub"] <= 45));
  EXPECT_TRUE((cmap["cpuc"] >= 25) && (cmap["cpuc"] <= 45));
}

TEST(CpuLoadBalancerRouteTest, AllZeroLoadsTwoChoice) {
  std::unordered_map<std::string, double> mymap;
  mymap.emplace("cpua", 0);
  mymap.emplace("cpub", 0);
  mymap.emplace("cpuc", 0);
  std::vector<std::shared_ptr<TestRouteHandleIf>> testHandles{
      makeRouteHandle<TestRouteHandleIf, TestRoute>("cpua", mymap),
      makeRouteHandle<TestRouteHandleIf, TestRoute>("cpub", mymap),
      makeRouteHandle<TestRouteHandleIf, TestRoute>("cpuc", mymap)};

  TestRouteHandle<LoadBalancerRoute<TestRouterInfo>> rh(
      testHandles,
      "TEST-SALT",
      std::chrono::milliseconds(100),
      /* fauloverCount */ 1,
      LoadBalancerRoute<TestRouterInfo>::AlgorithmType::TWO_RANDOM_CHOICES,
      /* fixed seed */ 0);

  std::unordered_map<std::string, size_t> cmap;
  for (int i = 0; i < 100; i++) {
    auto reply = rh.route(McGetRequest("0" + std::to_string(i)));
    std::string v = carbon::valueRangeSlow(reply).str();
    auto it = cmap.find(v);
    if (it != cmap.end()) {
      cmap[std::string(v)]++;
    } else {
      cmap.emplace(std::string(v), 1);
    }
  }
  LOG(INFO) << cmap["cpua"] << " " << cmap["cpub"] << " " << cmap["cpuc"];
  EXPECT_TRUE((cmap["cpua"] >= 25) && (cmap["cpua"] <= 45));
  EXPECT_TRUE((cmap["cpub"] >= 25) && (cmap["cpub"] <= 45));
  EXPECT_TRUE((cmap["cpuc"] >= 25) && (cmap["cpuc"] <= 45));
}

TEST(CpuLoadBalancerRouteTest, LoadsWithWaitTwoChoice) {
  std::unordered_map<std::string, double> mymap;
  mymap.emplace("cpua", 100);
  mymap.emplace("cpub", 50);
  mymap.emplace("cpuc", 50);
  std::vector<std::shared_ptr<TestRouteHandleIf>> testHandles{
      makeRouteHandle<TestRouteHandleIf, TestRoute>("cpua", mymap),
      makeRouteHandle<TestRouteHandleIf, TestRoute>("cpub", mymap),
      makeRouteHandle<TestRouteHandleIf, TestRoute>("cpuc", mymap)};

  TestRouteHandle<LoadBalancerRoute<TestRouterInfo>> rh(
      testHandles,
      "TEST-SALT",
      std::chrono::milliseconds(100),
      /* fauloverCount */ 1,
      LoadBalancerRoute<TestRouterInfo>::AlgorithmType::TWO_RANDOM_CHOICES,
      /* fixed seed */ 0);

  std::unordered_map<std::string, size_t> cmap;
  for (int i = 0; i < 100; i++) {
    auto reply = rh.route(McGetRequest("0" + std::to_string(i)));
    std::string v = carbon::valueRangeSlow(reply).str();
    auto it = cmap.find(v);
    if (it != cmap.end()) {
      cmap[std::string(v)]++;
    } else {
      cmap.emplace(std::string(v), 1);
    }
    // sleep here to cause server load of 'cpua' to expire
    if (i > 25) {
      /* sleep override */
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }
  LOG(INFO) << cmap["cpua"] << " " << cmap["cpub"] << " " << cmap["cpuc"];
  EXPECT_TRUE((cmap["cpua"] >= 10) && (cmap["cpua"] <= 20));
  EXPECT_TRUE((cmap["cpub"] >= 35) && (cmap["cpub"] <= 47));
  EXPECT_TRUE((cmap["cpuc"] >= 35) && (cmap["cpuc"] <= 47));
}
