/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ShardSplitter.h"

#include <folly/json/dynamic.h>

#include "mcrouter/lib/fbi/cpp/globals.h"
#include "mcrouter/lib/fbi/cpp/util.h"
#include "mcrouter/routes/ShardHashFunc.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

namespace {
constexpr size_t kMaxSplits = 26 * 26 + 1;
constexpr size_t kHostIdModulo = 16384;

size_t checkShardSplitSize(
    const folly::StringPiece shardId,
    const folly::dynamic& splitValue,
    folly::StringPiece name) {
  checkLogic(
      splitValue.isInt(),
      "ShardSplitter: {} is not an int for {}",
      name,
      shardId);
  auto split = splitValue.asInt();
  checkLogic(
      split > 0, "ShardSplitter: {} value is <= 0 for {}", name, shardId);
  if (static_cast<size_t>(split) > kMaxSplits) {
    LOG(ERROR) << "ShardSplitter: " << name << " value is greater than "
               << kMaxSplits << " for " << shardId;
    return kMaxSplits;
  }
  return static_cast<size_t>(split);
}

ShardSplitter::ShardSplitInfo parseSplit(
    const folly::dynamic& json,
    folly::StringPiece id,
    std::chrono::system_clock::time_point now) {
  checkLogic(
      json.isInt() || json.isObject(),
      "ShardSplitter: split config for {} is not an int or object",
      id);
  if (json.isInt()) {
    auto splitCnt = checkShardSplitSize(id, json, "shard_splits");
    return ShardSplitter::ShardSplitInfo(splitCnt);
  } else if (json.isObject()) {
    auto oldSplitJson = json.getDefault("old_split_size", 1);
    auto oldSplit = checkShardSplitSize(id, oldSplitJson, "old_split_size");
    auto newSplitJson = json.getDefault("new_split_size", 1);
    auto newSplit = checkShardSplitSize(id, newSplitJson, "new_split_size");
    auto startTimeJson = json.getDefault("split_start", 0);
    checkLogic(
        startTimeJson.isInt(),
        "ShardSplitter: split_start is not an int for {}",
        id);
    checkLogic(
        startTimeJson.asInt() >= 0,
        "ShardSplitter: split_start is negative for {}",
        id);
    std::chrono::system_clock::time_point startTime(
        std::chrono::seconds(startTimeJson.asInt()));
    auto migrationPeriodJson = json.getDefault("migration_period", 0);
    checkLogic(
        migrationPeriodJson.isInt(),
        "ShardSplitter: migration_period is not an int for {}",
        id);
    checkLogic(
        migrationPeriodJson.asInt() >= 0,
        "ShardSplitter: migration_period is negative for {}",
        id);
    std::chrono::duration<double> migrationPeriod(migrationPeriodJson.asInt());
    auto fanoutDeletesJson = json.getDefault("fanout_deletes", false);
    checkLogic(
        fanoutDeletesJson.isBool(),
        "ShardSplitter: fanout_deletes is not bool for {}",
        id);
    if (now > startTime + migrationPeriod || newSplit == oldSplit) {
      return ShardSplitter::ShardSplitInfo(
          newSplit, fanoutDeletesJson.asBool());
    } else {
      return ShardSplitter::ShardSplitInfo(
          oldSplit,
          newSplit,
          startTime,
          migrationPeriod,
          fanoutDeletesJson.asBool());
    }
  }
  // Should never reach here
  throwLogic("Invalid json type, not an int or object for {}", id);
  return ShardSplitter::ShardSplitInfo(1);
}

} // namespace

size_t ShardSplitter::ShardSplitInfo::getSplitSizeForCurrentHost() const {
  if (migrating_) {
    auto now = std::chrono::system_clock::now();
    if (now < startTime_) {
      return oldSplitSize_;
    } else if (now > startTime_ + migrationPeriod_) {
      migrating_ = false;
      return newSplitSize_;
    } else {
      double point = std::chrono::duration_cast<std::chrono::duration<double>>(
                         now - startTime_)
                         .count() /
          migrationPeriod_.count();
      return globals::hostid() % kHostIdModulo /
                  static_cast<double>(kHostIdModulo) <
              point
          ? newSplitSize_
          : oldSplitSize_;
    }
  }
  return newSplitSize_;
}

ShardSplitter::ShardSplitter(
    const folly::dynamic& json,
    const folly::dynamic& defaultSplitJson,
    bool enablePrefixMatching)
    : defaultShardSplit_(parseSplit(
          defaultSplitJson,
          "default",
          std::chrono::system_clock::now())),
      enablePrefixMatching_(enablePrefixMatching) {
  checkLogic(json.isObject(), "ShardSplitter: config is not an object");

  auto now = std::chrono::system_clock::now();
  shardSplits_.reserve(json.size());

  for (const auto& it : json.items()) {
    checkLogic(
        it.first.isString(),
        "ShardSplitter: expected string for key in shard split config, "
        "but got {}",
        it.first.typeName());
    folly::StringPiece shardId = it.first.getString();
    const auto split = parseSplit(it.second, shardId, now);
    // Only store if different than the default or is is migrating
    if (split.hasMigrationConfigured() ||
        split.getNewSplitSize() != defaultShardSplit_.getNewSplitSize()) {
      shardSplits_.emplace(shardId, split);
    }
  }
}

const ShardSplitter::ShardSplitInfo& ShardSplitter::getShardSplit(
    folly::StringPiece shard) const {
  for (;;) {
    auto splitIt = shardSplits_.find(shard);
    if (splitIt != shardSplits_.end()) {
      return splitIt->second;
    }

    if (!enablePrefixMatching_) {
      break;
    }

    // No match, lop off the last section and try again
    auto pos = shard.rfind('.');
    if (pos == std::string::npos) {
      break;
    }

    shard = shard.subpiece(0, pos);
  }
  return defaultShardSplit_;
}

const ShardSplitter::ShardSplitInfo* ShardSplitter::getShardSplit(
    folly::StringPiece routingKey,
    folly::StringPiece& shard) const {
  if (!getShardId(routingKey, shard)) {
    return nullptr;
  }
  return &getShardSplit(shard);
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
