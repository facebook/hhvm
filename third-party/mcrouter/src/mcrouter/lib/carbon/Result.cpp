/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/lib/carbon/Result.h"

#include <unordered_map>

namespace carbon {

namespace {
std::unordered_map<std::string, Result> makeStringToResultMap() {
  std::unordered_map<std::string, Result> resMap;
  for (size_t i = static_cast<size_t>(Result::UNKNOWN);
       i < static_cast<size_t>(Result::NUM_RESULTS);
       ++i) {
    auto res = static_cast<Result>(i);
    resMap[resultToString(res)] = res;
  }
  return resMap;
}

const static std::unordered_map<std::string, Result> kStringToResult =
    makeStringToResultMap();
} // anonymous namespace

Result resultFromString(const char* result) {
  auto it = kStringToResult.find(result);
  if (it != kStringToResult.cend()) {
    return it->second;
  }
  return Result::UNKNOWN;
}

} // namespace carbon
