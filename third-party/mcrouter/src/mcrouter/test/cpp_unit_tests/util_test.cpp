/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <string>

#include <gtest/gtest.h>
#include <sys/stat.h>

#include <folly/FileUtil.h>
#include <folly/experimental/TestUtil.h>

#include "mcrouter/lib/fbi/cpp/util.h"

using folly::test::TemporaryDirectory;
using folly::test::TemporaryFile;
using namespace facebook::memcache;

TEST(Util, ensure_has_permission_parity) {
  TemporaryFile tFile("util_test");
  std::string contents = "foo";
  std::string tPath(tFile.path().string());
  std::string badPath = "/bad_path" + tPath;

  // Get current file mode
  struct stat st;
  EXPECT_EQ(stat(tPath.data(), &st), 0);

  // Resetting the good file to the same mode should succeed for both APIs
  EXPECT_TRUE(ensureHasPermission(tPath.data(), st.st_mode));
  EXPECT_TRUE(
      !ensureHasPermissionOrReturnError(tPath.data(), st.st_mode).hasError());

  // Accessing the non-existent file should fail for both APIs
  EXPECT_FALSE(ensureHasPermission(badPath.data(), st.st_mode));
  EXPECT_FALSE(
      !ensureHasPermissionOrReturnError(badPath.data(), st.st_mode).hasError());
}

TEST(Util, ensure_dir_exists_and_writable_parity) {
  TemporaryDirectory tDir("util_test");
  std::string tPath(tDir.path().string());
  std::string badPath = "/bad_path" + tPath;

  // Ensuring the test dir created at start is writable
  EXPECT_TRUE(ensureDirExistsAndWritable(tPath.data()));
  EXPECT_TRUE(
      !ensureDirExistsAndWritableOrReturnError(tPath.data()).hasError());

  // Accessing the non-existent file should fail for both APIs
  EXPECT_FALSE(ensureDirExistsAndWritable(badPath.data()));
  EXPECT_FALSE(
      !ensureDirExistsAndWritableOrReturnError(badPath.data()).hasError());
}
