/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <unordered_map>

#include <folly/Range.h>

#include "mcrouter/CarbonRouterInstance.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

namespace detail {

bool getOptionsFromFlavor(
    folly::StringPiece flavorUri,
    std::unordered_map<std::string, std::string>&& optionOverrides,
    McrouterOptions& routerOptions);

} // namespace detail

template <class RouterInfo>
CarbonRouterInstance<RouterInfo>* createRouterFromFlavor(
    folly::StringPiece persistenceId,
    folly::StringPiece flavorUri,
    std::unordered_map<std::string, std::string> optionOverrides) {
  if (auto router = CarbonRouterInstance<RouterInfo>::get(persistenceId)) {
    return router;
  }

  McrouterOptions options;
  if (!detail::getOptionsFromFlavor(
          flavorUri, std::move(optionOverrides), options)) {
    return nullptr;
  }

  return CarbonRouterInstance<RouterInfo>::init(persistenceId, options);
}

template <class RouterInfo>
std::shared_ptr<CarbonRouterInstance<RouterInfo>> createRouterFromFlavor(
    folly::StringPiece flavorUri,
    std::unordered_map<std::string, std::string> optionOverrides) {
  McrouterOptions options;
  if (!detail::getOptionsFromFlavor(
          flavorUri, std::move(optionOverrides), options)) {
    return nullptr;
  }

  return CarbonRouterInstance<RouterInfo>::create(std::move(options));
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
