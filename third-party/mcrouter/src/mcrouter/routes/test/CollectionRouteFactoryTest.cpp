/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <vector>

#include <gtest/gtest.h>

#include <folly/Range.h>
#include <folly/json/json.h>

#include "mcrouter/CarbonRouterInstance.h"
#include "mcrouter/lib/carbon/example/gen/HelloGoodbyeRouterInfo.h"
#include "mcrouter/routes/test/AllSyncCollectionRouteFactory.h"
#include "mcrouter/routes/test/RouteHandleTestBase.h"
#include "mcrouter/routes/test/RouteHandleTestUtil.h"

using namespace facebook::memcache::mcrouter;
using namespace hellogoodbye;

namespace facebook {
namespace memcache {
namespace mcrouter {

class AllSyncCollectionRouteFactoryTest
    : public RouteHandleTestBase<HelloGoodbyeRouterInfo> {
 public:
  HelloGoodbyeRouterInfo::RouteHandlePtr getAllSyncCollectionRoute(
      folly::StringPiece jsonStr) {
    return createAllSyncCollectionRoute<HelloGoodbyeRouterInfo>(
        rhFactory_, folly::parseJson(jsonStr));
  }

  void testCreate(folly::StringPiece config) {
    TestFiberManager<HelloGoodbyeRouterInfo> fm;

    auto rh = getAllSyncCollectionRoute(config);
    ASSERT_TRUE(rh);
    EXPECT_EQ("AllSyncCollectionRoute", rh->routeName());

    fm.run([&rh]() {
      GoodbyeRequest req;
      auto reply = rh->route(req);
      EXPECT_TRUE(isErrorResult(*reply.result_ref()));
    });
  }
};

TEST_F(AllSyncCollectionRouteFactoryTest, create) {
  constexpr folly::StringPiece kSelectionRouteConfig = R"(
      {
        "type": "AllSyncRoute",
        "children": [
          "NullRoute",
          "ErrorRoute"
        ]
      }
    )";

  testCreate(kSelectionRouteConfig);
}

/* Tests a route handle that passes arguments down to the collector */
class ArgumentPassingCollectionRouteFactoryTest
    : public RouteHandleTestBase<HelloGoodbyeRouterInfo> {
 public:
  HelloGoodbyeRouterInfo::RouteHandlePtr getArgumentPassingCollectionRoute(
      folly::StringPiece jsonStr) {
    return createArgumentPassingCollectionRoute<HelloGoodbyeRouterInfo>(
        rhFactory_, folly::parseJson(jsonStr));
  }

  void testCreate(folly::StringPiece config) {
    TestFiberManager<HelloGoodbyeRouterInfo> fm;

    auto rh = getArgumentPassingCollectionRoute(config);
    ASSERT_TRUE(rh);
    EXPECT_EQ("ArgumentPassingCollectionRoute", rh->routeName());

    fm.run([&rh]() {
      GoodbyeRequest req;
      auto reply = rh->route(req);
      EXPECT_TRUE(isErrorResult(*reply.result_ref()));
    });
  }

 private:
  template <class RouterInfo>
  typename RouterInfo::RouteHandlePtr createArgumentPassingCollectionRoute(
      RouteHandleFactory<typename RouterInfo::RouteHandleIf>& factory,
      const folly::dynamic& json) {
    /* Basic test: pass hard coded string */
    const std::string hard_coded_string = "hard_coded_string";
    /* Pass hard coded complex type (std::vector) */
    const std::vector<std::string> hard_coded_vector = {"hard_coded_vector"};
    /* Pass value derived from JSON */
    std::string does_arg_passing_work;
    if (auto jDoesArgPassingWork = json.get_ptr("does_arg_passing_work")) {
      does_arg_passing_work = jDoesArgPassingWork->asString();
    }
    return createCollectionRoute<RouterInfo, ArgumentPassingCollectionRoute>(
        factory,
        json,
        hard_coded_string,
        hard_coded_vector,
        does_arg_passing_work);
  }

  template <class Request, class... Args>
  class ArgumentPassingCollector
      : public Collector<Request, ArgumentPassingCollector, Args...> {
    using Reply = ReplyT<Request>;
    using ParentCollector = Collector<Request, ArgumentPassingCollector>;

   public:
    ArgumentPassingCollector(
        const std::string& hard_coded_string_arg,
        const std::vector<std::string>& hard_coded_vector_arg,
        const std::string& does_arg_passing_work) {
      EXPECT_STREQ(hard_coded_string_arg.c_str(), "hard_coded_string");
      EXPECT_STREQ(hard_coded_vector_arg[0].c_str(), "hard_coded_vector");
      EXPECT_STREQ(does_arg_passing_work.c_str(), "yes");
    }

    folly::Optional<Reply> initialReplyImpl() const {
      return folly::none;
    }

    folly::Optional<Reply> iterImpl(const Reply& reply) {
      return reply;
    }

    Reply finalReplyImpl() const {
      throw std::logic_error("Should never reach here");
    }

    size_t getBatchSizeImpl() const {
      return ParentCollector::getChildrenCount();
    }
  };

  /* Tests ability to pass arguments to route */
  template <class RouterInfo, class... Args>
  class ArgumentPassingCollectionRoute
      : public CollectionRoute<RouterInfo, ArgumentPassingCollector, Args...> {
   public:
    std::string routeName() const {
      return "ArgumentPassingCollectionRoute";
    }
    explicit ArgumentPassingCollectionRoute(
        const std::vector<typename RouterInfo::RouteHandlePtr>& children,
        const Args&... arg)
        : CollectionRoute<RouterInfo, ArgumentPassingCollector, Args...>(
              children,
              arg...) {
      assert(!children.empty());
    }
  };
};

TEST_F(ArgumentPassingCollectionRouteFactoryTest, create) {
  constexpr folly::StringPiece kSelectionRouteConfig = R"(
      {
        "type": "ArgumentPassingCollectionRoute",
        "children": [
          "ErrorRoute",
          "NullRoute"
        ],
        "does_arg_passing_work": "yes"
      }
    )";

  testCreate(kSelectionRouteConfig);
}

} // end namespace mcrouter
} // end namespace memcache
} // end namespace facebook
