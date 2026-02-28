/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <string>

#include "mcrouter/lib/network/FailureDomains.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

uint32_t getFailureDomainHash(folly::StringPiece failureDomain) {
  return folly::hash::SpookyHashV2::Hash32(
      failureDomain.begin(), failureDomain.size(), 0);
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
