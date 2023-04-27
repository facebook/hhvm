/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Optional.h>
#include <folly/Range.h>

namespace facebook {
namespace memcache {

class McrouterOptions;

namespace mcrouter {

void mcrouterSetThisThreadName(
    const McrouterOptions& opts,
    folly::StringPiece prefix,
    folly::Optional<size_t> threadId = folly::none);
}
} // namespace memcache
} // namespace facebook
