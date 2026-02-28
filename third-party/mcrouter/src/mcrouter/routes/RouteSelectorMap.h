/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>

#include <folly/container/F14Map.h>

namespace facebook {
namespace memcache {
namespace mcrouter {

template <class RouteHandleIf>
class PrefixSelectorRoute;

template <class RouteHandleIf>
using RouteSelectorMap = folly::F14NodeMap<
    std::string,
    std::shared_ptr<PrefixSelectorRoute<RouteHandleIf>>>;
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
