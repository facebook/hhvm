/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstdint>

#include <folly/Range.h>

namespace facebook {
namespace memcache {

/**
 * Struct that holds information about a given destination in a pool.
 */
struct PoolContext {
  folly::StringPiece poolName;
  size_t indexInPool;
  bool isShadow;
};

} // namespace memcache
} // namespace facebook
