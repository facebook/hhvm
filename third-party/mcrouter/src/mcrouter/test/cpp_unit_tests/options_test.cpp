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

  dict.clear();
  dict["enable_tw_crash_config_backup_path"] = "1";
  e = opts.updateFromDict(dict);
  EXPECT_TRUE(e.empty());
  EXPECT_TRUE(opts.enable_tw_crash_config_backup_path);
}

TEST(OptionsSetFromDictTest, StringMapValueMayContainColons) {
  McrouterOptions opts;
  const unordered_map<string, string> dict{
      {"config_params", "proc-name:ray::RayTrainWorker,environment:prod"}};

  const auto errors = opts.updateFromDict(dict);

  EXPECT_TRUE(errors.empty());
  EXPECT_EQ(opts.config_params.at("proc-name"), "ray::RayTrainWorker");
  EXPECT_EQ(opts.config_params.at("environment"), "prod");
}

TEST(OptionsSetFromDictTest, StringMapRejectsMalformedPairs) {
  for (const auto& value : {"missing-delimiter", ":missing-name"}) {
    McrouterOptions opts;
    opts.config_params = {{"existing", "value"}};
    const unordered_map<string, string> dict{{"config_params", value}};

    const auto errors = opts.updateFromDict(dict);

    ASSERT_EQ(errors.size(), 1);
    EXPECT_EQ(errors[0].requestedName, "config_params");
    EXPECT_EQ(errors[0].requestedValue, value);
    EXPECT_EQ(
        opts.config_params,
        (unordered_map<string, string>{{"existing", "value"}}));
  }
}
