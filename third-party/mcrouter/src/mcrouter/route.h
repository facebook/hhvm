/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Range.h>

namespace facebook {
namespace memcache {
namespace mcrouter {

/**
 * True if pattern (like "/foo/a*c/") matches a route (like "/foo/abc")
 */
bool match_pattern_route(folly::StringPiece pattern, folly::StringPiece route);
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
