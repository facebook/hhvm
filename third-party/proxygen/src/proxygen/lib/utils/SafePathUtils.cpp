/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "SafePathUtils.h"

#include <algorithm>
#include <cerrno>
#include <filesystem>
#include <fmt/format.h>
#include <optional>
#include <string>

#include <folly/String.h>
#include <folly/container/Foreach.h>
#include <folly/portability/Stdlib.h>

#include <boost/filesystem/path.hpp>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#ifndef MAX_LONG_PATH
#define MAX_LONG_PATH 32767
#endif

using namespace std;

namespace proxygen {

std::string SafePath::getPath(const std::string& filePath,
                              const std::vector<std::string>& allowlist,
                              bool useRealPaths) {
  if (allowlist.empty()) {
    throw std::runtime_error("Allowlist is empty!");
  }

  auto normalizedPathExpected = getNormalizedPathSafe(filePath);
  if (normalizedPathExpected.hasError()) {
    normalizedPathExpected.error().throw_exception();
  }
  auto normalizedPath = std::move(normalizedPathExpected).value();

  // If enabled, make sure all paths are real paths to avoid a false mismatch.
  std::optional<std::string> maybeRealFilePath;
  std::optional<std::vector<std::string>> maybeRealAllowlist;
  if (useRealPaths) {
    auto realPathExpected = getRealPathSafe(filePath);
    if (realPathExpected.hasError()) {
      realPathExpected.error().throw_exception();
    }
    maybeRealFilePath = std::move(realPathExpected).value();

    std::vector<std::string> realAllowlist;
    realAllowlist.reserve(allowlist.size());
    for (const auto& path : allowlist) {
      auto realPath = getRealPathSafe(path);
      if (realPath.hasError()) {
        realPath.error().throw_exception();
      }
      realAllowlist.emplace_back(std::move(realPath).value());
    }
    maybeRealAllowlist.emplace(std::move(realAllowlist));
  }

  auto finalFilePath = maybeRealFilePath.value_or(normalizedPath);
  auto it = maybeRealAllowlist
                ? std::find(maybeRealAllowlist.value().begin(),
                            maybeRealAllowlist.value().end(),
                            finalFilePath)
                : std::find(allowlist.begin(), allowlist.end(), finalFilePath);
  auto mismatch = maybeRealAllowlist ? it == maybeRealAllowlist.value().end()
                                     : it == allowlist.end();
  if (mismatch) {
    throw std::runtime_error(
        fmt::format("File path={} doesn't match with any of the allowlisted "
                    "values, used real paths={}, normalized path={}",
                    finalFilePath,
                    useRealPaths,
                    normalizedPath));
  }
  return normalizedPath;
}

folly::Expected<std::string, folly::exception_wrapper> SafePath::getPathSafe(
    const std::string& filePath,
    const std::string& baseDirectory,
    bool useRealPaths) {
  if (baseDirectory.empty()) {
    return folly::makeUnexpected(
        folly::make_exception_wrapper<std::runtime_error>(
            "Base directory is empty!"));
  }

  auto normalizedPathExpected = getNormalizedPathSafe(filePath);
  if (normalizedPathExpected.hasError()) {
    return folly::makeUnexpected(std::move(normalizedPathExpected).error());
  }
  auto normalizedPath = std::move(normalizedPathExpected).value();

  // If enabled, make sure all paths are real paths to avoid a false mismatch.
  std::optional<std::string> maybeRealFilePath;
  std::optional<std::string> maybeRealBaseDirectory;
  if (useRealPaths) {
    auto realFilePathExpected = getRealPathSafe(filePath);
    if (realFilePathExpected.hasError()) {
      return folly::makeUnexpected(std::move(realFilePathExpected).error());
    }
    maybeRealFilePath = std::move(realFilePathExpected).value();

    auto realBaseDirectoryExpected = getRealPathSafe(baseDirectory);
    if (realBaseDirectoryExpected.hasError()) {
      return folly::makeUnexpected(
          std::move(realBaseDirectoryExpected).error());
    }
    maybeRealBaseDirectory = std::move(realBaseDirectoryExpected).value();
  }

  auto finalFilePath = maybeRealFilePath.value_or(normalizedPath);
  auto finalBaseDirectory = maybeRealBaseDirectory.value_or(baseDirectory);
  if (!startsWithBaseDir(finalFilePath, finalBaseDirectory)) {
    return folly::makeUnexpected(
        folly::make_exception_wrapper<std::runtime_error>(
            fmt::format("File path={} doesn't start with the base "
                        "directory={}, used real paths={}, normalized path={}",
                        finalFilePath,
                        finalBaseDirectory,
                        useRealPaths,
                        normalizedPath)));
  }

  return folly::makeExpected<folly::exception_wrapper>(
      std::move(normalizedPath));
}

std::string SafePath::getPath(const std::string& filePath,
                              const std::string& baseDirectory,
                              bool useRealPaths) {
  auto result = getPathSafe(filePath, baseDirectory, useRealPaths);
  if (UNLIKELY(result.hasError())) {
    result.error().throw_exception();
  }

  return std::move(result).value();
}

string SafePath::cleanPath(const string& path) {
  boost::filesystem::path output;
  boost::filesystem::path inPath(path);
  FOR_EACH_RANGE(it, inPath.begin(), inPath.end()) {
    if (*it == ".") {
      continue;
    }
    if (*it == "..") {
      if (output.filename().string() != "/") {
        output.remove_filename();
      }
      continue;
    }
    output /= *it;
  }
  output.make_preferred();
  return output.string();
}

folly::Expected<std::string, folly::exception_wrapper>
SafePath::getNormalizedPathSafe(const std::string& filePath) {
  // cleanPath removes all ../, ./ references from file path
  // For example, if file path is "/tmp/../etc/passwd", cleanPath returns
  // "/etc/passwd"
  std::string normalizedPath = cleanPath(filePath);

  if (normalizedPath == "") {
    return folly::makeUnexpected(
        folly::make_exception_wrapper<std::runtime_error>(fmt::format(
            "Normalized file path is empty, original path={}", filePath)));
  }

  // Preventing null byte injection. "/etc/passwd\0.png" => "/etc/passwd"
  std::string resultPath(normalizedPath.c_str());

  // path length can not exceed maximum path size
#ifdef _WIN32
  std::filesystem::path p(normalizedPath);
  if (p.is_relative()) {
    // From
    // https://docs.microsoft.com/en-us/windows/win32/fileio/maximum-file-path-limitation
    // "Because you cannot use the "\\?\" prefix with a relative path,
    // relative paths are always limited to a total of PATH_MAX characters."
    if (resultPath.length() > PATH_MAX) {
      return folly::makeUnexpected(folly::make_exception_wrapper<
                                   std::runtime_error>(fmt::format(
          "Normalized file path={} is too long, original path={}, path_max={}",
          normalizedPath,
          filePath,
          PATH_MAX)));
    }
  } else {
    if (resultPath.length() > MAX_LONG_PATH) {
      return folly::makeUnexpected(folly::make_exception_wrapper<
                                   std::runtime_error>(fmt::format(
          "Normalized file path={} is too long, original path={}, path_max={}",
          normalizedPath,
          filePath,
          MAX_LONG_PATH)));
    }
  }
#else
  if (resultPath.length() > PATH_MAX) {
    return folly::makeUnexpected(folly::make_exception_wrapper<
                                 std::runtime_error>(fmt::format(
        "Normalized file path={} is too long, original path={}, path_max={}",
        normalizedPath,
        filePath,
        PATH_MAX)));
  }
#endif

  return resultPath;
}

bool SafePath::startsWithBaseDir(const std::string& filePath,
                                 const std::string& baseDirectory) {
  if (filePath == baseDirectory) {
    return true;
  }

  // Adding "/" at the end of provided base directory so that input like
  // "/tmp/root_sensitive" can not get bypassed where base directory is
  // "/tmp/root"
  auto pathSeparator =
      static_cast<char>(std::filesystem::path::preferred_separator);
  std::string tempBaseDir;
  tempBaseDir.reserve(baseDirectory.size() + 1);
  tempBaseDir.append(baseDirectory);
  if (tempBaseDir.back() != pathSeparator) {
    tempBaseDir += pathSeparator;
  }

  return filePath.find(tempBaseDir) == 0;
}

folly::Expected<std::string, folly::exception_wrapper>
SafePath::getRealPathSafe(const std::string& filePath) {
  char* result;
#ifdef _WIN32
  char buffer[MAX_LONG_PATH];
  result = _fullpath(buffer, filePath.c_str(), MAX_LONG_PATH);
#else
  char buffer[PATH_MAX];
  result = realpath(filePath.c_str(), buffer);
#endif
  if (result == nullptr) {
    return folly::makeUnexpected(
        folly::make_exception_wrapper<std::runtime_error>(
            fmt::format("Unable to read real path={}, errno={}",
                        filePath,
                        folly::errnoStr(errno))));
  }
  std::string absolutePath(buffer);

  return absolutePath;
}

} // namespace proxygen
