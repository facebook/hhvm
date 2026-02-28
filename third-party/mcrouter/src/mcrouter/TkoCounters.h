/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <atomic>
#include <cstddef>

namespace facebook {
namespace memcache {
namespace mcrouter {

/**
 * Number of boxes marked as TKO
 */
struct TkoCounters {
  std::atomic<size_t> softTkos{0};
  std::atomic<size_t> hardTkos{0};

  size_t totalTko() const {
    return softTkos + hardTkos;
  }
};

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
