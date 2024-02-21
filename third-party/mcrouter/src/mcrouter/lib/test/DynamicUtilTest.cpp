/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/json/dynamic.h>
#include <folly/portability/GTest.h>

#include "mcrouter/lib/DynamicUtil.h"

using facebook::memcache::mcrouter::searchDynamic;

TEST(DynamicUtil, searchDynamic) {
  folly::dynamic haystack = folly::dynamic::object;
  haystack["array"] = folly::dynamic::array(
      "bar", "baz", folly::dynamic::array("bletch", "xyzzy"));
  haystack[""] = 0;
  haystack["one"] = 1;
  haystack["str"] = "this is a string";
  haystack["obj"] = folly::dynamic::object;
  haystack["obj"]["first"] = folly::dynamic::object("nested", "abc");
  haystack["obj"]["second"] = folly::dynamic::array(1, 2, 3);
  haystack["long_array"] =
      folly::dynamic::array(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
  haystack["b"] = true;

  auto* res = searchDynamic(haystack, "not found");
  EXPECT_EQ(nullptr, res);

  res = searchDynamic(haystack, "this is a string");
  EXPECT_NE(nullptr, res);
  EXPECT_EQ(haystack.get_ptr("str"), res);

  res = searchDynamic(
      haystack,
      folly::dynamic::array(
          "bar", "baz", folly::dynamic::array("bletch", "xyzzy")));
  EXPECT_NE(nullptr, res);
  EXPECT_EQ(haystack.get_ptr("array"), res);

  res = searchDynamic(haystack, folly::dynamic::array(1, 2, 3));
  EXPECT_NE(nullptr, res);
  EXPECT_EQ(haystack["obj"].get_ptr("second"), res);

  res = searchDynamic(haystack, true);
  EXPECT_NE(nullptr, res);
  EXPECT_EQ(haystack.get_ptr("b"), res);

  res = searchDynamic(haystack, false);
  EXPECT_EQ(nullptr, res);

  // multiple occurrences: will return pointer to the first match.
  res = searchDynamic(haystack, 2);
  EXPECT_NE(nullptr, res);
  EXPECT_EQ(2, res->asInt());
}
