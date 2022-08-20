/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <string>
#include <unordered_map>

#include <gtest/gtest.h>

#include <folly/File.h>
#include <folly/FileUtil.h>
#include <folly/experimental/TestUtil.h>

#include "mcrouter/flavor.h"
#include "mcrouter/lib/fbi/cpp/util.h"

using facebook::memcache::mcrouter::readFlavor;
using folly::test::TemporaryFile;

std::string eraseStr(std::string str, const std::string& substr) {
  return str.erase(str.find(substr), substr.size());
}

TEST(Flavor, readStandaloneFlavor) {
  // Write temporary flavor file to test.
  TemporaryFile flavorFile("web-standalone");
  std::string flavorContents =
      "{"
      "\"libmcrouter_options\": {"
      "\"default_route\": \"abc\" /* comment */, "
      "\"thrift_compression_threshold\": \"768\""
      "},"
      "\"standalone_options\": {"
      "\"port\": \"11001\", "
      "\"log_file\": \"mcrouter.log\""
      "}"
      "}";
  std::string flavorPath(flavorFile.path().string());
  EXPECT_EQ(
      folly::writeFull(
          flavorFile.fd(), flavorContents.data(), flavorContents.size()),
      flavorContents.size());

  // Reads the flavor file to test.
  std::unordered_map<std::string, std::string> libmcrouter_opts;
  std::unordered_map<std::string, std::string> standalone_opts;

  std::string flavor = eraseStr(flavorPath, "-standalone");
  EXPECT_TRUE(readFlavor(flavor, standalone_opts, libmcrouter_opts));

  EXPECT_EQ(4, libmcrouter_opts.size());
  EXPECT_EQ(1, libmcrouter_opts.count("default_route"));
  EXPECT_EQ("abc", libmcrouter_opts["default_route"]);
  EXPECT_EQ(1, libmcrouter_opts.count("router_name"));
  EXPECT_EQ("web", libmcrouter_opts["router_name"]);
  EXPECT_EQ(1, libmcrouter_opts.count("flavor_name"));
  EXPECT_EQ(flavor, libmcrouter_opts["flavor_name"]);
  EXPECT_EQ("768", libmcrouter_opts["thrift_compression_threshold"]);
  EXPECT_EQ(2, standalone_opts.size());
  EXPECT_EQ(1, standalone_opts.count("port"));
  EXPECT_EQ("11001", standalone_opts["port"]);
  EXPECT_EQ(1, standalone_opts.count("log_file"));
  EXPECT_EQ("mcrouter.log", standalone_opts["log_file"]);
}

TEST(Flavor, readLibmcrouterFlavor) {
  // Write temporary libmcrouter flavor file to test.
  TemporaryFile flavorFile("web");
  std::string flavorContents =
      "{"
      "\"options\": {"
      "\"default_route\": \"abc\", "
      "\"thrift_compression_threshold\": \"768\""
      "}"
      "}";
  std::string flavorPath(flavorFile.path().string());
  EXPECT_EQ(
      folly::writeFull(
          flavorFile.fd(), flavorContents.data(), flavorContents.size()),
      flavorContents.size());

  // Reads the flavor file to test.
  std::unordered_map<std::string, std::string> libmcrouter_opts;
  std::unordered_map<std::string, std::string> standalone_opts;

  EXPECT_TRUE(readFlavor(flavorPath, standalone_opts, libmcrouter_opts));

  EXPECT_EQ(0, standalone_opts.size());
  EXPECT_EQ(4, libmcrouter_opts.size());
  EXPECT_EQ(1, libmcrouter_opts.count("default_route"));
  EXPECT_EQ("abc", libmcrouter_opts["default_route"]);
  EXPECT_EQ(1, libmcrouter_opts.count("router_name"));
  EXPECT_EQ("web", libmcrouter_opts["router_name"]);
  EXPECT_EQ(1, libmcrouter_opts.count("flavor_name"));
  EXPECT_EQ(flavorPath, libmcrouter_opts["flavor_name"]);
  EXPECT_EQ("768", libmcrouter_opts["thrift_compression_threshold"]);
}

TEST(Flavor, readFlavorFromTwoFiles) {
  // Write temporary standalone flavor file to test.
  TemporaryFile flavorStandaloneFile("web-standalone");
  std::string flavorStandaloneContents =
      "{"
      "\"standalone_options\": {"
      "\"port\": \"11001\", /* comment */"
      "\"log_file\": \"mcrouter.log\""
      "}"
      "}";
  std::string flavorStandalonePath(flavorStandaloneFile.path().string());
  EXPECT_EQ(
      folly::writeFull(
          flavorStandaloneFile.fd(),
          flavorStandaloneContents.data(),
          flavorStandaloneContents.size()),
      flavorStandaloneContents.size());

  // Write temporary libmcrouter flavor file to test.
  std::string flavorPath(eraseStr(flavorStandalonePath, "-standalone"));
  folly::File flavorFile(flavorPath.data(), O_RDWR | O_CREAT);
  std::string flavorContents =
      "{"
      "\"options\": {"
      "\"default_route\": \"abc\""
      "}"
      "}";
  EXPECT_EQ(
      folly::writeFull(
          flavorFile.fd(), flavorContents.data(), flavorContents.size()),
      flavorContents.size());

  // Reads the flavor file to test. Expects that we read from both files.
  std::unordered_map<std::string, std::string> libmcrouter_opts;
  std::unordered_map<std::string, std::string> standalone_opts;

  EXPECT_TRUE(readFlavor(flavorPath, standalone_opts, libmcrouter_opts));

  EXPECT_EQ(3, libmcrouter_opts.size());
  EXPECT_EQ(1, libmcrouter_opts.count("default_route"));
  EXPECT_EQ("abc", libmcrouter_opts["default_route"]);
  EXPECT_EQ(1, libmcrouter_opts.count("router_name"));
  EXPECT_EQ("web", libmcrouter_opts["router_name"]);
  EXPECT_EQ(1, libmcrouter_opts.count("flavor_name"));
  EXPECT_EQ(flavorPath, libmcrouter_opts["flavor_name"]);
  EXPECT_EQ(2, standalone_opts.size());
  EXPECT_EQ(1, standalone_opts.count("port"));
  EXPECT_EQ("11001", standalone_opts["port"]);
  EXPECT_EQ(1, standalone_opts.count("log_file"));
  EXPECT_EQ("mcrouter.log", standalone_opts["log_file"]);
}

TEST(Flavor, readFlavorFromTwoFilesShouldOverrideLibmcrouterOptions) {
  // Write temporary standalone flavor file to test.
  TemporaryFile flavorStandaloneFile("web-standalone");
  std::string flavorStandaloneContents =
      "{"
      "\"libmcrouter_options\": {"
      "\"default_route\": \"def\""
      "},"
      "\"standalone_options\": {"
      "\"port\": \"11001\", "
      "\"log_file\": \"mcrouter.log\""
      "}"
      "}";
  std::string flavorStandalonePath(flavorStandaloneFile.path().string());
  EXPECT_EQ(
      folly::writeFull(
          flavorStandaloneFile.fd(),
          flavorStandaloneContents.data(),
          flavorStandaloneContents.size()),
      flavorStandaloneContents.size());

  // Write temporary libmcrouter flavor file to test.
  std::string flavorPath(eraseStr(flavorStandalonePath, "-standalone"));
  folly::File flavorFile(flavorPath.data(), O_RDWR | O_CREAT);
  std::string flavorContents =
      "{"
      "\"options\": {"
      "// comment\n"
      "\"default_route\": \"abc\""
      "}"
      "}";
  EXPECT_EQ(
      folly::writeFull(
          flavorFile.fd(), flavorContents.data(), flavorContents.size()),
      flavorContents.size());

  // Reads the flavor file to test. Expects that we read from both files.
  std::unordered_map<std::string, std::string> libmcrouter_opts;
  std::unordered_map<std::string, std::string> standalone_opts;

  EXPECT_TRUE(readFlavor(flavorPath, standalone_opts, libmcrouter_opts));

  EXPECT_EQ(3, libmcrouter_opts.size());
  EXPECT_EQ(1, libmcrouter_opts.count("default_route"));
  EXPECT_EQ("def", libmcrouter_opts["default_route"]);
  EXPECT_EQ(1, libmcrouter_opts.count("router_name"));
  EXPECT_EQ("web", libmcrouter_opts["router_name"]);
  EXPECT_EQ(1, libmcrouter_opts.count("flavor_name"));
  EXPECT_EQ(flavorPath, libmcrouter_opts["flavor_name"]);
  EXPECT_EQ(2, standalone_opts.size());
  EXPECT_EQ(1, standalone_opts.count("port"));
  EXPECT_EQ("11001", standalone_opts["port"]);
  EXPECT_EQ(1, standalone_opts.count("log_file"));
  EXPECT_EQ("mcrouter.log", standalone_opts["log_file"]);
}

TEST(Flavor, readFlavorShouldReportMalformedStandaloneFlavor) {
  // Write temporary standalone flavor file to test.
  TemporaryFile flavorStandaloneFile("web-standalone");
  std::string flavorStandaloneContents =
      "{"
      "\"standalone_options\": {"
      "\"port\": \"11001\", "
      "\"log_file\": \"mcrouter.log\""
      "},"
      "\"libmcrouter_options\": ["
      "--i-have-no-idea-what-i'm-doing"
      "]"
      "}";
  std::string flavorStandalonePath(flavorStandaloneFile.path().string());
  EXPECT_EQ(
      folly::writeFull(
          flavorStandaloneFile.fd(),
          flavorStandaloneContents.data(),
          flavorStandaloneContents.size()),
      flavorStandaloneContents.size());

  // Write temporary libmcrouter flavor file to test.
  std::string flavorPath(eraseStr(flavorStandalonePath, "-standalone"));
  folly::File flavorFile(flavorPath.data(), O_RDWR | O_CREAT);
  std::string flavorContents =
      "{"
      "\"options\": {"
      "\"default_route\": \"abc\""
      "}"
      "}";
  EXPECT_EQ(
      folly::writeFull(
          flavorFile.fd(), flavorContents.data(), flavorContents.size()),
      flavorContents.size());

  // Reads the flavor file to test. Expects that we read from both files.
  std::unordered_map<std::string, std::string> libmcrouter_opts;
  std::unordered_map<std::string, std::string> standalone_opts;

  EXPECT_FALSE(readFlavor(flavorPath, standalone_opts, libmcrouter_opts));
}

TEST(Flavor, readFlavorShouldReportMalformedLibmcrouterFlavor) {
  // Write temporary standalone flavor file to test.
  TemporaryFile flavorStandaloneFile("web-standalone");
  std::string flavorStandaloneContents =
      "{"
      "\"standalone_options\": {"
      "\"port\": \"11001\", "
      "\"log_file\": \"mcrouter.log\""
      "}"
      "}";
  std::string flavorStandalonePath(flavorStandaloneFile.path().string());
  EXPECT_EQ(
      folly::writeFull(
          flavorStandaloneFile.fd(),
          flavorStandaloneContents.data(),
          flavorStandaloneContents.size()),
      flavorStandaloneContents.size());

  // Write temporary libmcrouter flavor file to test.
  std::string flavorPath(eraseStr(flavorStandalonePath, "-standalone"));
  folly::File flavorFile(flavorPath.data(), O_RDWR | O_CREAT);
  std::string flavorContents =
      "{"
      "\"libmcrouter_options\": {"
      "\"default_route\": \"abc\""
      "}"
      "}";
  EXPECT_EQ(
      folly::writeFull(
          flavorFile.fd(), flavorContents.data(), flavorContents.size()),
      flavorContents.size());

  // Reads the flavor file to test. Expects that we read from both files.
  std::unordered_map<std::string, std::string> libmcrouter_opts;
  std::unordered_map<std::string, std::string> standalone_opts;

  EXPECT_FALSE(readFlavor(flavorPath, standalone_opts, libmcrouter_opts));
}
