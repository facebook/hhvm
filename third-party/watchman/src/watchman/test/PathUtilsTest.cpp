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
    struct stat st{};
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
  folly::fs::create_directories(state_dir);

  // Verify the state directory already exists
  EXPECT_TRUE(folly::fs::exists(state_dir));

  auto expected = fmt::format("{}/{}", state_dir, suffix);

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

#ifndef _WIN32
TEST_F(PathUtilsTest, computeFileNameWithSymlinkToExistingDirectory) {
  std::string result;
  std::string suffix = getRandomSuffix();

  auto actual_state_dir = getTempPath() + "/watchman_state_actual";
  folly::fs::create_directories(actual_state_dir);

  // Verify the state directory already exists
  EXPECT_TRUE(folly::fs::exists(actual_state_dir));

  auto state_dir = getTempPath() + "/watchman_state";
  folly::fs::create_symlink(actual_state_dir, state_dir);
  setWatchmanStateDir(state_dir);

  // Verify the state directory exists and is a symlink
  EXPECT_TRUE(folly::fs::exists(state_dir));
  EXPECT_TRUE(folly::fs::is_symlink(state_dir));

  auto expected = fmt::format("{}/{}", state_dir, suffix);

  // This should compute a path and create directories as needed
  EXPECT_NO_THROW({
    compute_file_name(result, "testuser", suffix.c_str(), "testfile", true);
  });

  // The result should not be empty
  EXPECT_FALSE(result.empty());
  EXPECT_EQ(result, expected);

  // Verify the directory was created
  EXPECT_TRUE(folly::fs::exists(state_dir));

  // The result should be an absolute path and have correct permissions
  EXPECT_TRUE(result[0] == '/');
  checkDirectoryPermissions(state_dir);
}
#endif

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

// create_log_dir tests — exercises the log-specific directory creation
// that skips verify_dir_ownership (unlike create_state_dir)

TEST_F(PathUtilsTest, createLogDirCreatesNewDirectory) {
  // Parent must exist (e.g. set up by container runtime); only leaf is created
  auto parent = getTempPath() + "/watchman_logs";
  folly::fs::create_directories(parent);
  auto log_dir = parent + "/testuser";
  EXPECT_FALSE(folly::fs::exists(log_dir));

  std::error_code ec;
  create_log_dir(log_dir.c_str(), ec);

  EXPECT_FALSE(ec) << "create_log_dir should succeed: " << ec.message();
  EXPECT_TRUE(folly::fs::exists(log_dir));
  EXPECT_TRUE(folly::fs::is_directory(log_dir));

#ifndef _WIN32
  struct stat st{};
  ASSERT_EQ(stat(log_dir.c_str(), &st), 0);
  // Log dirs get 0755, not 0700 like state dirs
  mode_t perms = st.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);
  EXPECT_EQ(perms, static_cast<mode_t>(0755))
      << fmt::format("Expected 0755 but got 0{:o}", perms);
#endif
}

TEST_F(PathUtilsTest, createLogDirFailsWhenParentMissing) {
  // Parent directory must exist — create_log_dir only creates the leaf.
  // A missing parent indicates a misconfigured log_dir and should fail
  // clearly rather than silently creating a deep tree.
  auto log_dir = getTempPath() + "/nonexistent_parent/watchman_logs";
  EXPECT_FALSE(folly::fs::exists(getTempPath() + "/nonexistent_parent"));

  std::error_code ec;
  create_log_dir(log_dir.c_str(), ec);

  EXPECT_TRUE(ec) << "create_log_dir should fail when parent doesn't exist";
  EXPECT_FALSE(folly::fs::exists(log_dir));
}

TEST_F(PathUtilsTest, createLogDirHandlesExistingDirectory) {
  // Pre-existing directory (e.g. container runtime pre-created it)
  auto log_dir = getTempPath() + "/watchman_logs";
  folly::fs::create_directories(log_dir);
  ASSERT_TRUE(folly::fs::exists(log_dir));

  std::error_code ec;
  create_log_dir(log_dir.c_str(), ec);

  // Should succeed without error even though dir already exists
  EXPECT_FALSE(ec) << "create_log_dir should handle existing dir: "
                   << ec.message();
  EXPECT_TRUE(folly::fs::exists(log_dir));
}

#ifndef _WIN32
TEST_F(PathUtilsTest, createLogDirHandlesSymlinkToExistingDirectory) {
  // Symlink to an existing directory should not cause errors
  auto actual_dir = getTempPath() + "/watchman_logs_actual";
  folly::fs::create_directories(actual_dir);
  ASSERT_TRUE(folly::fs::exists(actual_dir));

  auto log_dir = getTempPath() + "/watchman_logs";
  folly::fs::create_symlink(actual_dir, log_dir);
  ASSERT_TRUE(folly::fs::is_symlink(log_dir));

  std::error_code ec;
  create_log_dir(log_dir.c_str(), ec);

  EXPECT_FALSE(ec) << "create_log_dir should handle symlinks: " << ec.message();
  EXPECT_TRUE(folly::fs::exists(log_dir));
}

TEST_F(PathUtilsTest, createLogDirDoesNotExitOnDifferentOwnership) {
  // Key difference from create_state_dir: create_log_dir must not call
  // exit(1) when the directory has different ownership or permissions.
  // In containers, the parent may be owned by root.
  // We verify this by creating a dir with permissive mode (which would
  // fail verify_dir_ownership's 0022 check) and confirming create_log_dir
  // succeeds.
  auto log_dir = getTempPath() + "/watchman_logs_permissive";
  folly::fs::create_directories(log_dir);
  chmod(log_dir.c_str(), 0777);

  std::error_code ec;
  create_log_dir(log_dir.c_str(), ec);

  // create_log_dir should not exit or error — it doesn't check ownership
  EXPECT_FALSE(ec) << "create_log_dir should not fail on permissive dir: "
                   << ec.message();
  EXPECT_TRUE(folly::fs::exists(log_dir));
}
#endif

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
