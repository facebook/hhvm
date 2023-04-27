/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <string>

#include <gtest/gtest.h>

#include <folly/FileUtil.h>
#include <folly/experimental/TestUtil.h>

#include "mcrouter/ConfigApi.h"
#include "mcrouter/options.h"

using facebook::memcache::McrouterOptions;
using facebook::memcache::mcrouter::ConfigApi;
using facebook::memcache::mcrouter::ConfigType;
using folly::test::TemporaryFile;

TEST(ConfigApi, file_change) {
  TemporaryFile config("config_api_test");
  std::string contents = "a";
  std::string path(config.path().string());

  EXPECT_EQ(
      folly::writeFull(config.fd(), contents.data(), contents.size()),
      contents.size());

  McrouterOptions opts;
  opts.config = "file:" + path;
  ConfigApi api(opts);
  api.startObserving();

  std::atomic<int> changes(0);
  auto handle = api.subscribe([&changes]() { ++changes; });

  api.trackConfigSources();
  std::string buf;
  std::string outPath;
  EXPECT_TRUE(api.get(ConfigType::ConfigFile, path, buf));
  EXPECT_EQ(contents, buf);

  EXPECT_TRUE(api.getConfigFile(buf, outPath));
  EXPECT_EQ(contents, buf);
  EXPECT_EQ("file:" + path, outPath);
  api.subscribeToTrackedSources();

  EXPECT_EQ(changes, 0);

  contents = "b";
  EXPECT_EQ(
      folly::writeFull(config.fd(), contents.data(), contents.size()),
      contents.size());

  // wait for the file to flush and api to check for update
  sleep(4);

  EXPECT_EQ(changes, 1);

  EXPECT_TRUE(api.getConfigFile(buf, outPath));
  EXPECT_EQ("ab", buf);
  EXPECT_EQ("file:" + path, outPath);

  EXPECT_TRUE(api.get(ConfigType::ConfigFile, path, buf));
  EXPECT_EQ("ab", buf);

  // clear tracked sources
  api.trackConfigSources();
  api.subscribeToTrackedSources();

  contents = "c";
  EXPECT_EQ(
      folly::writeFull(config.fd(), contents.data(), contents.size()),
      contents.size());

  // wait for the file to flush
  sleep(4);

  EXPECT_EQ(changes, 1);

  EXPECT_TRUE(api.getConfigFile(buf, outPath));
  EXPECT_EQ("abc", buf);
  EXPECT_EQ("file:" + path, outPath);

  EXPECT_TRUE(api.get(ConfigType::ConfigFile, path, buf));
  EXPECT_EQ("abc", buf);

  api.stopObserving(getpid());
}
