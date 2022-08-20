/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "CarbonRouterFactory.h"

#include <unordered_map>

#include "mcrouter/McrouterLogFailure.h"
#include "mcrouter/config.h"

namespace facebook {
namespace memcache {
namespace mcrouter {
namespace detail {

bool getOptionsFromFlavor(
    folly::StringPiece flavorUri,
    std::unordered_map<std::string, std::string>&& optionOverrides,
    McrouterOptions& routerOptions) {
  std::unordered_map<std::string, std::string> optionsMap;
  if (!readLibmcrouterFlavor(flavorUri, optionsMap)) {
    return false;
  }

  for (auto& it : optionOverrides) {
    optionsMap[it.first] = std::move(it.second);
  }

  auto errors = routerOptions.updateFromDict(optionsMap);
  for (const auto& err : errors) {
    MC_LOG_FAILURE(
        routerOptions,
        failure::Category::kInvalidOption,
        "Option parse error: {}={}, {}",
        err.requestedName,
        err.requestedValue,
        err.errorMsg);
  }
  return true;
}

} // namespace detail
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
