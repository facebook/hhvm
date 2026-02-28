/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <chrono>

#include <folly/Range.h>
#include <folly/json/dynamic.h>

#include "mcrouter/config.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

class ShardSplitter {
 public:
  class ShardSplitInfo {
   public:
    explicit ShardSplitInfo(size_t splits, bool fanoutDeletes = true)
        : oldSplitSize_(splits),
          newSplitSize_(splits),
          migrationPeriod_{0},
          fanoutDeletes_(fanoutDeletes),
          migrating_(false) {}

    ShardSplitInfo(
        size_t oldSplitSize,
        size_t newSplitSize,
        std::chrono::system_clock::time_point startTime,
        std::chrono::duration<double> migrationPeriod,
        bool fanoutDeletes)
        : oldSplitSize_(oldSplitSize),
          newSplitSize_(newSplitSize),
          startTime_(startTime),
          migrationPeriod_(migrationPeriod),
          fanoutDeletes_(fanoutDeletes),
          migrating_(true) {}

    bool fanoutDeletesEnabled() const {
      return fanoutDeletes_;
    }
    size_t getOldSplitSize() const {
      return oldSplitSize_;
    }
    size_t getNewSplitSize() const {
      return newSplitSize_;
    }
    size_t getSplitSizeForCurrentHost() const;

    bool hasMigrationConfigured() const {
      return migrating_;
    }

   private:
    const size_t oldSplitSize_;
    const size_t newSplitSize_;
    const std::chrono::system_clock::time_point startTime_;
    const std::chrono::duration<double> migrationPeriod_;
    const bool fanoutDeletes_;
    mutable bool migrating_;
  };

  explicit ShardSplitter(
      const folly::dynamic& json,
      const folly::dynamic& defaultSplitJson = 1,
      bool enablePrefixMatching = false);

  /**
   * Returns information about shard split if it exists. If it does, stores
   * shardId part of the key in shardId.
   *
   * @return  nullptr if key doesn't have valid shard id, or if there is no
   *          shard split found. Otherwise returns pointer to ShardSplitInfo.
   */
  const ShardSplitInfo* getShardSplit(
      folly::StringPiece key,
      folly::StringPiece& shardId) const;

  /**
   * Returns information about shard split given shard id.
   *
   * @return  Returns reference to ShardSplitInfo.
   */
  const ShardSplitInfo& getShardSplit(folly::StringPiece shardId) const;

  using ShardSplitInfoMap = folly::F14FastMap<std::string, ShardSplitInfo>;

  const ShardSplitInfoMap& getShardSplits() const {
    return shardSplits_;
  }

 private:
  ShardSplitInfoMap shardSplits_;
  ShardSplitInfo defaultShardSplit_;
  bool enablePrefixMatching_;
};
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
