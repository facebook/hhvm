/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/SanityCheck.h"

#include <folly/io/FsUtil.h>
#include <folly/portability/GTest.h>
#include <folly/portability/Unistd.h>
#include <folly/testing/TestUtil.h>

#include <climits>
#include <fstream>
#include <string>
#include <string_view>

using folly::test::TemporaryDirectory;
using namespace watchman;

class SanityCheckTest : public ::testing::Test {
 protected:
  std::string path(std::string_view name) const {
    return (tempDir_.path() / std::string{name}).string();
  }

  bool writeFile(const std::string& filePath, std::string_view contents) {
    std::ofstream file(filePath);
    file << contents;
    file.close();
    return file.good();
  }

  TemporaryDirectory tempDir_;
};

TEST_F(SanityCheckTest, CheckExecutableChange_SameInode_ReturnsUnchanged) {
  const auto executable = path("watchman");
  ASSERT_TRUE(writeFile(executable, "first"));

  const auto identity = getIdentityForPath(executable.c_str());

  ASSERT_TRUE(identity.has_value());
  EXPECT_EQ(
      checkExecutableChange(*identity, executable.c_str()),
      ExecutableChange::Unchanged);
}

TEST_F(SanityCheckTest, CheckExecutableChange_ReplacedFile_ReturnsChanged) {
  const auto executable = path("watchman");
  const auto replacement = path("watchman.new");
  ASSERT_TRUE(writeFile(executable, "first"));
  ASSERT_TRUE(writeFile(replacement, "second"));
  const auto identity = getIdentityForPath(executable.c_str());
  ASSERT_TRUE(identity.has_value());

  folly::fs::rename(replacement, executable);

  EXPECT_EQ(
      checkExecutableChange(*identity, executable.c_str()),
      ExecutableChange::Changed);
}

TEST_F(SanityCheckTest, CheckExecutableChange_MissingPath_ReturnsUnknown) {
  const auto executable = path("watchman");
  ASSERT_TRUE(writeFile(executable, "first"));
  const auto identity = getIdentityForPath(executable.c_str());
  ASSERT_TRUE(identity.has_value());

  folly::fs::remove(executable);

  EXPECT_EQ(
      checkExecutableChange(*identity, executable.c_str()),
      ExecutableChange::Unknown);
}

TEST_F(SanityCheckTest, GetIdentityForPath_ProcSelfExe_MatchesResolvedPath) {
  char buffer[PATH_MAX];
  const auto length = ::readlink("/proc/self/exe", buffer, sizeof(buffer));
  ASSERT_GT(length, 0);
  ASSERT_LT(static_cast<size_t>(length), sizeof(buffer));
  const std::string resolved(buffer, length);

  const auto viaMagicLink = getIdentityForPath("/proc/self/exe");
  const auto viaResolvedPath = getIdentityForPath(resolved.c_str());

  ASSERT_TRUE(viaMagicLink.has_value());
  ASSERT_TRUE(viaResolvedPath.has_value());
  EXPECT_EQ(*viaMagicLink, *viaResolvedPath);
}
