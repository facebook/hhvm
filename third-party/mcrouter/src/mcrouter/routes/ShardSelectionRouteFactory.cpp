/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ShardSelectionRouteFactory.h"

#include <algorithm>
#include <cassert>

#include <folly/Random.h>
#include <folly/json/dynamic.h>

namespace facebook {
namespace memcache {
namespace mcrouter {

namespace detail {

namespace {

std::vector<size_t> parseShardsJsonArray(const folly::dynamic& shardsJson) {
  assert(shardsJson.isArray());

  std::vector<size_t> shards;
  shards.reserve(shardsJson.size());

  for (const auto& shardIdJson : shardsJson) {
    checkLogic(
        shardIdJson.isInt(),
        "ShardSelectionRoute: 'shards' property expected to be an "
        "array of integers. Invalid shard found in array: {}",
        shardIdJson);
    shards.push_back(static_cast<size_t>(shardIdJson.asInt()));
  }

  return shards;
}

std::vector<size_t> parseShardsJsonString(const folly::dynamic& shardsJson) {
  assert(shardsJson.isString());

  std::vector<size_t> shards;

  auto shardsStr = shardsJson.stringPiece();
  while (!shardsStr.empty()) {
    auto shardId = shardsStr.split_step(',');
    try {
      shards.push_back(folly::to<size_t>(shardId));
    } catch (const std::exception& e) {
      throwLogic(
          "ShardSelectionRoute: 'shards' property expected to be a string of "
          "comma-separated integers. Invalid shard found in string: {}. "
          "Exception: {}",
          shardId,
          e.what());
    }
  }

  return shards;
}

} // namespace

std::vector<std::vector<size_t>> parseAllShardsJson(
    const folly::dynamic& allShardsJson) {
  assert(allShardsJson.isArray());

  std::vector<std::vector<size_t>> allShards;
  allShards.reserve(allShardsJson.size());

  size_t i = 0;
  for (const auto& shardsJson : allShardsJson) {
    if (shardsJson.isArray()) {
      allShards.push_back(parseShardsJsonArray(shardsJson));
    } else if (shardsJson.isString()) {
      allShards.push_back(parseShardsJsonString(shardsJson));
    } else {
      throwLogic(
          "ShardSelectionRoute: 'shards[{}]' must be an array of integers or a "
          "string of comma-separated shard ids.",
          i);
    }
    ++i;
  }

  return allShards;
}

std::vector<size_t> parseShardsPerServerJson(const folly::dynamic& jShards) {
  if (jShards.isArray()) {
    return parseShardsJsonArray(jShards);
  } else if (jShards.isString()) {
    return parseShardsJsonString(jShards);
  } else {
    throwLogic(
        "EagerShardSelectionRoute: 'shards[...]' must be an array of "
        "integers or a string of comma-separated shard ids.");
  }
}

} // namespace detail

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
