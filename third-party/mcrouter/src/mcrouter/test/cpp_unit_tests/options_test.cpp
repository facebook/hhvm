/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <string>
#include <unordered_map>
#include <vector>

#include <gtest/gtest.h>

#include "mcrouter/options.h"

using facebook::memcache::McrouterOptions;
using std::string;
using std::unordered_map;
using std::vector;

TEST(OptionsSetFromDictTest, sanity) {
  McrouterOptions opts;
  unordered_map<string, string> dict;

  auto e = opts.updateFromDict(dict);
  EXPECT_TRUE(e.empty());

  /* default */
  EXPECT_TRUE(opts.num_proxies == 1);

  dict["num_proxies"] = "4";
  e = opts.updateFromDict(dict);
  EXPECT_TRUE(e.empty());
  EXPECT_TRUE(opts.num_proxies == 4);

  dict.clear();
  dict["num_proxies"] = "a";
  e = opts.updateFromDict(dict);
  EXPECT_EQ(e.size(), 1);
  EXPECT_EQ(e[0].requestedName, "num_proxies");
  EXPECT_EQ(e[0].requestedValue, "a");
  /* unchanged */
  EXPECT_TRUE(opts.num_proxies == 4);

  dict.clear();
  dict["blah"] = "a";
  e = opts.updateFromDict(dict);
  /* unknown options don't cause errors */
  EXPECT_EQ(e.size(), 0);
  /* unchanged */
  EXPECT_TRUE(opts.num_proxies == 4);
}
