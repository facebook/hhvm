/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "mcrouter/config.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

/* Time Provider Func for Migrate route */
class TimeProviderFunc {
 public:
  time_t operator()() const {
    return nowWallSec();
  }
};
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
