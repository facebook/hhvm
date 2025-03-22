/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "BigValueRoute.h"

#include <folly/Format.h>
#include <folly/fibers/WhenN.h>

#include "mcrouter/lib/fbi/cpp/ParsingUtil.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

namespace detail {

// Hashes value on a separate CPU thread pool, preempts fiber until hashing is
// complete.
uint64_t hashBigValue(const folly::IOBuf& value) {
  if (auto singleton = AuxiliaryCPUThreadPoolSingleton::try_get_fast()) {
    auto& threadPool = singleton->getThreadPool();
    return folly::fibers::await([&](folly::fibers::Promise<uint64_t> promise) {
      threadPool.add([promise = std::move(promise), &value]() mutable {
        auto hash = folly::IOBufHash()(value);
        // Note: for compatibility with old code running in production we're
        // using only 32-bits of hash.
        promise.setValue(hash & ((1ull << 32) - 1));
      });
    });
  }
  throwRuntime(
      "Mcrouter CPU Thread pool is not running, cannot calculate hash for big "
      "value!");
}

BigValueRouteOptions parseBigValueRouteSettings(const folly::dynamic& json) {
  auto jThreshold = json.get_ptr("split_threshold");
  checkLogic(
      jThreshold != nullptr, "BigValueRoute: 'split_threshold' is missing");
  size_t threshold = parseInt(
      *jThreshold, "split_threshold", 0, std::numeric_limits<int64_t>::max());

  auto jBatchSize = json.get_ptr("batch_size");
  checkLogic(jBatchSize != nullptr, "BigValueRoute: 'batch_size' is missing");
  size_t batchSize = parseInt(
      *jBatchSize, "batch_size", 0, std::numeric_limits<int64_t>::max());

  bool hideReplyFlags = false;
  if (auto jHideReplyFlag = json.get_ptr("hide_reply_flag")) {
    hideReplyFlags = parseBool(*jHideReplyFlag, "hide_reply_flag");
  }

  return BigValueRouteOptions{
      /*threshold_=*/threshold,
      /*batchSize_=*/batchSize,
      /*hideReplyFlags_=*/hideReplyFlags};
}

} // namespace detail

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
