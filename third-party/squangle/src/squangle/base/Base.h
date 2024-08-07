/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/container/F14Map.h>
#include <chrono>
#include <string>

namespace facebook::common::mysql_client {

using Duration = std::chrono::duration<uint64_t, std::micro>;
using Timepoint = std::chrono::time_point<std::chrono::steady_clock>;

// The extra StringHasher and std::equal_to<> makes this unordered map
// "transparent", meaning we can .find() by std::string, std::string_view, and
// by const char*. */
using AttributeMap = folly::F14NodeMap<std::string, std::string>;

} // namespace facebook::common::mysql_client
