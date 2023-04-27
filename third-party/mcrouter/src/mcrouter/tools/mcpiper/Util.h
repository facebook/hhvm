/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <sys/time.h>
#include <string>
#include <vector>

#include <boost/regex.hpp>

namespace facebook {
namespace memcache {

/**
 * Return a list of strings that describes the flags present in
 * "flags" argument.
 */
std::vector<std::string> describeFlags(uint64_t flags);

std::string printTimeAbsolute(const struct timeval& ts);
std::string printTimeDiff(const struct timeval& ts, struct timeval& prev);
std::string printTimeOffset(const struct timeval& ts, struct timeval& prev);

/**
 * Builds a boost::regex for the given pattern and settings.
 * Note: Will call exit(1) if the pattern is invalid.
 *
 * @return  boost::regex or nullptr if pattern is nullptr.
 */
std::unique_ptr<boost::regex> buildRegex(
    const std::string& pattern,
    bool ignoreCase = false) noexcept;
} // namespace memcache
} // namespace facebook
