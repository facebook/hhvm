/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/routes/test/BigValueRouteTestBase.h"

TEST(BigValueRouteTest, smallvalue) {
  facebook::memcache::mcrouter::testSmallvalue<
      facebook::memcache::MemcacheRouterInfo>();
}

TEST(BigValueRouteTest, bigvalueWithFlag) {
  facebook::memcache::mcrouter::testBigvalueWithFlag<
      facebook::memcache::MemcacheRouterInfo>();
}

TEST(BigValueRouteTest, bigvalueWithoutFlag) {
  facebook::memcache::mcrouter::testBigvalueWithoutFlag<
      facebook::memcache::MemcacheRouterInfo>();
}

TEST(BigValueRouteTest, bigvalue) {
  facebook::memcache::mcrouter::testBigvalue<
      facebook::memcache::MemcacheRouterInfo>();
}
