/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <string>

#include <gtest/gtest.h>

#include "mcrouter/lib/McResUtil.h"

namespace facebook {
namespace memcache {
namespace test {

TEST(McResUtil, carbon_resultFromString) {
  const char* resStr1 = "mc_res_busy";
  ASSERT_EQ(carbon::resultFromString(resStr1), carbon::Result::BUSY);

  const char* resStr2 = "bad_string";
  ASSERT_EQ(carbon::resultFromString(resStr2), carbon::Result::UNKNOWN);

  std::string resStr3 = "mc_res_notfound";
  ASSERT_EQ(
      carbon::resultFromString(resStr3.c_str()), carbon::Result::NOTFOUND);
}

} // namespace test
} // namespace memcache
} // namespace facebook
