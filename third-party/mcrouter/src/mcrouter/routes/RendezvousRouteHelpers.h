/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Range.h>
#include <folly/json/dynamic.h>
#include <string>
#include <vector>

namespace facebook {
namespace memcache {
namespace mcrouter {

std::vector<folly::StringPiece> getTags(
    const folly::dynamic& json,
    size_t numRoutes,
    const std::string& nameForErrorMessage);

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
