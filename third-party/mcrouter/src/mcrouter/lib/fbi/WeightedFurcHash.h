/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Range.h>

namespace facebook {
namespace mcrouter {

uint32_t weightedFurcHash(
    folly::StringPiece key,
    folly::Range<const double*> weights,
    uint32_t maxRetries = 32);

} // namespace mcrouter
} // namespace facebook
