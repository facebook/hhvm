/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/PathUtils.h"

#include <folly/Exception.h>
#include <folly/FileUtil.h>
#include <folly/Random.h>
#include <folly/portability/GTest.h>
#include <folly/portability/Stdlib.h>
#include <folly/portability/SysTypes.h>
#include <folly/portability/Unistd.h>
#include <folly/test/TestUtils.h>
#include <folly/testing/TestUtil.h>
#include <string>

#ifndef _WIN32
#include <sys/stat.h>
#endif

using folly::test::TemporaryDirectory;
using namespace watchman;

class PathUtilsTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Create a temporary directory for testing
    tempDir_ = std::make_unique<TemporaryDirectory>();
  }

  void TearDown() override {
    tempDir_.reset();
  }

  // Helper function to set up environment for testing
  void setWatchmanStateDir(const std::string& stateDirPath) {
    // Set environment variables that affect state directory computation
    setenv("WATCHMAN_TEST_STATE_DIR", stateDirPath.c_str(), 1);
  }

  std::string getTempPath() const {
    return tempDir_->path().string();
  }

  std::string getRandomSuffix() const {
    return fmt::format("testsuffix{}", folly::Random::rand32());
  }

  std::unique_ptr<TemporaryDirectory> tempDir_;

  // Helper function to check permissions on a directory (Unix only)
  void checkDirectoryPermissions(const std::string& dir_path) {
#ifndef _WIN32
    struct stat st {};
    EXPECT_EQ(stat(dir_path.c_str(), &st), 0)
        << fmt::format("Should be able to stat directory: {}", dir_path);

    // Verify the directory is owned by the current effective user
    // This check applies to all Unix systems (verify_dir_ownership checks this)
    EXPECT_EQ(st.st_uid, geteuid()) << fmt::format(
        "Directory {} should be owned by current effective user", dir_path);

    // Check that group and other don't have write permissions
    // This matches the security check in verify_dir_ownership (st.st_mode &
    // 0022)
    EXPECT_EQ(st.st_mode & S_IWGRP, 0) << fmt::format(
        "Directory {} should not have group write permissions", dir_path);
    EXPECT_EQ(st.st_mode & S_IWOTH, 0) << fmt::format(
        "Directory {} should not have other write permissions", dir_path);

#ifdef __APPLE__
    // On macOS, we can be more strict and expect exactly 0700 permissions
    mode_t expected_perms = S_IRWXU; // 0700
    mode_t actual_perms = st.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);

    EXPECT_EQ(actual_perms, expected_perms) << fmt::format(
        "Directory {} should have 0700 permissions, but got 0{:o}",
        dir_path,
        actual_perms);
#endif
    // Note: On Linux, the exact permissions may vary due to ACLs and config
    // but the security requirements (owner-only write access) are still
    // enforced
#else
    // Windows
    (void)dir_path;
#endif
  }
};

TEST_F(PathUtilsTest, computeFileNameWithExistingDirectory) {
  std::string result;
  std::string suffix = getRandomSuffix();

  auto state_dir = getTempPath() + "/watchman_state";
  setWatchmanStateDir(state_dir);

  auto expected = fmt::format("{}/{}", state_dir, suffix);

  // Verify the state directory doesn't exist initially
  EXPECT_FALSE(folly::fs::exists(state_dir));

  // This should compute a path and create directories as needed
  EXPECT_NO_THROW({
    compute_file_name(result, "testuser", suffix.c_str(), "testfile", true);
  });

  // The result should not be empty
  EXPECT_FALSE(result.empty());
  EXPECT_EQ(result, expected);

  // Verify the directory was created
  EXPECT_TRUE(folly::fs::exists(state_dir));

  // The result should be an absolute path (on Unix systems) and have correct
  // permissions
#ifndef _WIN32
  EXPECT_TRUE(result[0] == '/');
  checkDirectoryPermissions(state_dir);
#endif
}

TEST_F(PathUtilsTest, computeFileNameWithNonExistingDirectory) {
  std::string result;
  std::string suffix = getRandomSuffix();

  auto state_dir_parent = getTempPath() + "watchman_state/a/b";

  // Verify that a parent directory doesn't exist
  EXPECT_FALSE(folly::fs::exists(state_dir_parent));

  auto state_dir = state_dir_parent + "/c/d";
  setWatchmanStateDir(state_dir);

  auto expected = fmt::format("{}/{}", state_dir, suffix);

  // This should compute a path and create directories as needed
  EXPECT_NO_THROW({
    compute_file_name(result, "testuser", suffix.c_str(), "testfile", true);
  });

  // The result should not be empty
  EXPECT_FALSE(result.empty());
  EXPECT_EQ(result, expected);

  // Verify the leaf directory was created
  EXPECT_TRUE(folly::fs::exists(state_dir));

  // The result should be an absolute path (on Unix systems) and have correct
  // permissions
#ifndef _WIN32
  EXPECT_TRUE(result[0] == '/');
  checkDirectoryPermissions(state_dir);
#endif
}

TEST_F(PathUtilsTest, computeFileNameRelativePathRequireAbsolute) {
  std::string result = "relative/path";
  std::string suffix = getRandomSuffix();

  // This should cause a FATAL error on Unix systems due to non-absolute path
#ifndef _WIN32
  EXPECT_DEATH(
      {
        compute_file_name(result, "testuser", suffix.c_str(), "testfile", true);
      },
      "must be an absolute file path");
#else
  // On Windows, this should not cause an error
  EXPECT_NO_THROW({
    compute_file_name(result, "testuser", suffix.c_str(), "testfile", true);
  });
#endif
}

TEST_F(PathUtilsTest, computeFileNameRelativePathNoRequireAbsolute) {
  std::string result = "relative/path";
  std::string suffix = getRandomSuffix();

  // When require_absolute is false, relative paths should be allowed
  EXPECT_NO_THROW({
    compute_file_name(result, "testuser", suffix.c_str(), "testfile", false);
  });

  EXPECT_EQ(result, "relative/path");
}

TEST_F(PathUtilsTest, computeFileNameDifferentSuffixes) {
  auto state_dir = getTempPath() + "/watchman_state";
  setWatchmanStateDir(state_dir);

  // Verify the state directory doesn't exist initially
  EXPECT_FALSE(folly::fs::exists(state_dir));

  // Test various suffixes that are used in watchman
  std::vector<std::pair<std::string, std::string>> suffixTests = {
      {"pid", "pidfile"},
      {"sock", "sockname"},
      {"state", "statefile"},
      {"log", "logfile"}};

  for (const auto& [suffix, what] : suffixTests) {
    std::string result;
    std::string expected = fmt::format("{}/{}", state_dir, suffix);

    EXPECT_NO_THROW({
      compute_file_name(result, "testuser", suffix.c_str(), what.c_str(), true);
    }) << fmt::format("Failed for suffix: {}, what: {}", suffix, what);

    EXPECT_FALSE(result.empty())
        << fmt::format("Empty result for suffix: {}, what: {}", suffix, what);

    EXPECT_EQ(result, expected) << fmt::format(
        "Result doesn't match expected: {}, what: {}", suffix, what);
  }
}
