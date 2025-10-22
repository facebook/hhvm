/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <string>

#include "mcrouter/lib/fbi/cpp/LogFailure.h"

namespace facebook {
namespace memcache {

class McrouterOptions;

namespace mcrouter {

std::string routerName(const McrouterOptions& opts);

#define MC_LOG_FAILURE(opts, ...) LOG_FAILURE(routerName(opts), __VA_ARGS__)
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
