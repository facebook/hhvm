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
namespace test {

// Generate n endpoints.
std::pair<std::vector<std::string>, std::vector<folly::StringPiece>>
genEndpoints(int n);

// Generate weight json for endpoints with the value weights.
folly::dynamic genWeights(const std::vector<double>& weights);

} // namespace test
} // namespace memcache
} // namespace facebook
