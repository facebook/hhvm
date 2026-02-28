/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/MapUtil.h"
#include <folly/portability/GTest.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "watchman/watchman_string.h"

using watchman::mapContainsAny;
using watchman::mapContainsAnyOf;

TEST(MapUtil, map_contains_any) {
  std::unordered_map<w_string, int> uMap = {
      {"one", 1}, {"two", 2}, {"three", 3}};

  // Map contains key

  EXPECT_TRUE(mapContainsAny(uMap, "one"))
      << "mapContainsAny single string present";

  EXPECT_TRUE(mapContainsAny(uMap, "one", "two"))
      << "mapContainsAny two strings present";

  EXPECT_TRUE(mapContainsAny(uMap, "one", "two", "three"))
      << "mapContainsAny three strings present";

  EXPECT_TRUE(mapContainsAny(uMap, "one", "xcase"))
      << "mapContainsAny first string present";

  EXPECT_TRUE(mapContainsAny(uMap, "xcase", "two"))
      << "mapContainsAny second string present";

  EXPECT_TRUE(mapContainsAny(uMap, "xcase1", "xcase2", "three"))
      << "mapContainsAny last string present";

  // Map does not contain key

  EXPECT_FALSE(mapContainsAny(uMap, "xcase"))
      << "mapContainsAny single string absent";

  EXPECT_FALSE(mapContainsAny(uMap, "xcase1", "xcase2"))
      << "mapContainsAny two strings absent";

  EXPECT_FALSE(mapContainsAny(uMap, "xcase1", "xcase2", "xcase3"))
      << "mapContainsAny three strings absent";

  // Empty map tests
  std::unordered_map<w_string, w_string> eMap;

  EXPECT_FALSE(mapContainsAny(eMap, "xcase1"))
      << "mapContainsAny absent on empty map";
}

TEST(MapUtil, map_contains_any_of) {
  std::unordered_map<w_string, int> uMap = {
      {"one", 1}, {"two", 2}, {"three", 3}};

  {
    // Using iterator to do the lookup
    std::unordered_set<w_string> kSet = {"one"};
    EXPECT_TRUE(mapContainsAnyOf(uMap, kSet.begin(), kSet.end()))
        << "mapContainsAnyOf single string present";

    kSet.emplace("two");
    EXPECT_TRUE(mapContainsAnyOf(uMap, kSet.begin(), kSet.end()))
        << "mapContainsAnyOf two strings present";

    kSet.emplace("three");
    EXPECT_TRUE(mapContainsAnyOf(uMap, kSet.begin(), kSet.end()))
        << "mapContainsAnyOf all strings present";
  }
  {
    std::unordered_set<w_string> kSet = {"one", "xcase1", "xcase2", "xcase3"};
    EXPECT_TRUE(mapContainsAnyOf(uMap, kSet.begin(), kSet.end()))
        << "mapContainsAnyOf one of several strings present";

    kSet.emplace("two");
    EXPECT_TRUE(mapContainsAnyOf(uMap, kSet.begin(), kSet.end()))
        << "mapContainsAnyOf two of several strings present";
  }
  {
    std::unordered_set<w_string> kSet;
    EXPECT_FALSE(mapContainsAnyOf(uMap, kSet.begin(), kSet.end()))
        << "mapContainsAnyOf empty set";

    kSet.emplace("xcase1");
    EXPECT_FALSE(mapContainsAnyOf(uMap, kSet.begin(), kSet.end()))
        << "mapContainsAnyOf single string absent";

    kSet.emplace("xcase2");
    EXPECT_FALSE(mapContainsAnyOf(uMap, kSet.begin(), kSet.end()))
        << "mapContainsAnyOf two strings absent";

    kSet.emplace("xcase3");
    EXPECT_FALSE(mapContainsAnyOf(uMap, kSet.begin(), kSet.end()))
        << "mapContainsAnyOf three strings absent";
  }

  // Empty map tests
  {
    std::unordered_map<w_string, w_string> eMap;

    std::unordered_set<w_string> kSet;
    EXPECT_FALSE(mapContainsAnyOf(eMap, kSet.begin(), kSet.end()))
        << "mapContainsAnyOf absent on empty map and set";

    kSet.emplace("one");
    EXPECT_FALSE(mapContainsAnyOf(eMap, kSet.begin(), kSet.end()))
        << "mapContainsAnyOf absent on empty map and non-empty set";

    kSet.emplace("two");
    EXPECT_FALSE(mapContainsAnyOf(eMap, kSet.begin(), kSet.end()))
        << "mapContainsAnyOf absent on empty map and 2 item set";
  }
}
