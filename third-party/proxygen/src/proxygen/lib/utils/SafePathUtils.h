/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <algorithm>
#include <string>

#include <folly/ExceptionWrapper.h>
#include <folly/Expected.h>

namespace proxygen {

/**
 * This class provides some APIs for cleaning and validating file path to
 * prevent path traversal attack
 */
class SafePath {
 public:
  /**
   * Checking against whitelist of permitted values
   * - normalizes file path
   * - checks if the normalized path matches with any of the whitelisted values
   * - if useRealPaths is enabled, then the file path's real path will be used
   * for validation
   * - throws error if the normalized path doesn't belong to the whitelist
   */
  static std::string getPath(const std::string& filePath,
                             const std::vector<std::string>& whitelist,
                             bool useRealPaths = false);

  /**
   * Checking prefix and suffix of file path
   * - normalizes file path
   * - checks if the normalized path starts with the given base directory
   * - if useRealPaths is enabled, then the file path's real path will be used
   * for validation
   * - throws error if the prefix and/or suffix dont match
   */
  static std::string getPath(const std::string& filePath,
                             const std::string& baseDirectory,
                             bool useRealPaths = false);

  /**
   * Checking prefix and suffix of file path
   * - normalizes file path
   * - checks if the normalized path starts with the given base directory
   * - if useRealPaths is enabled, then the file path's real path will be used
   * for validation
   * - returns unexpected variant if the prefix and/or suffix dont match
   */
  static folly::Expected<std::string, folly::exception_wrapper> getPathSafe(
      const std::string& filePath,
      const std::string& baseDirectory,
      bool useRealPaths = false);

  /**
   * Helper functions have visibility "private" so that these can not be called
   * from outside. We dont want them to be used standalone because they don't
   * provide complete protection.
   */
 private:
  static folly::Expected<std::string, folly::exception_wrapper>
  getNormalizedPathSafe(const std::string& filePath);

  static folly::Expected<std::string, folly::exception_wrapper> getRealPathSafe(
      const std::string& filePath);

  static bool startsWithBaseDir(const std::string& filePath,
                                const std::string& baseDirectory);

  /**
   * Remove '.' components, '..' components along with the preceding
   * component if any, duplicate '/'s and trailing '/'.
   */
  static std::string cleanPath(const std::string& path);
};

} // namespace proxygen
