/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ShardSplitRoute.h"

#include "mcrouter/lib/config/RouteHandleFactory.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

std::string shardSplitSuffix(size_t offset) {
  if (!offset) {
    return "";
  }

  std::string ret;
  ret.push_back('a' + ((offset - 1) % 26));
  ret.push_back('a' + ((offset - 1) / 26));
  return ret;
}

namespace detail {

std::string createSplitKey(
    folly::StringPiece fullKey,
    size_t offset,
    folly::StringPiece shard) {
  if (!offset) {
    return fullKey.str();
  }

  std::string newKey;
  newKey.reserve(fullKey.size() + 2);
  newKey.append(fullKey.begin(), shard.end());
  newKey.push_back('a' + ((offset - 1) % 26));
  newKey.push_back('a' + ((offset - 1) / 26));
  newKey.append(shard.end(), fullKey.end());
  return newKey;
}

} // namespace detail

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
