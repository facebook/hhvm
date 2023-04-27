/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include <utility>
#include <vector>

#include "mcrouter/routes/McrouterRouteHandle.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

class ShadowSettings;

template <class RouterInfo>
using ShadowData = std::vector<std::pair<
    std::shared_ptr<typename RouterInfo::RouteHandleIf>,
    std::shared_ptr<ShadowSettings>>>;

using McrouterShadowData = ShadowData<McrouterRouterInfo>;

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
