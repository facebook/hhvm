/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Range.h>
#include <string>

namespace facebook {
namespace memcache {
namespace mcrouter {

uint32_t getFailureDomainHash(folly::StringPiece failureDomain);

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
