/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Range.h>
#include <folly/futures/Future.h>
#include <folly/io/IOBuf.h>

namespace fizz {
namespace server {

enum ReplayCacheResult {
  NotChecked,
  NotReplay,
  MaybeReplay,
  DefinitelyReplay,
};
}

folly::StringPiece toString(server::ReplayCacheResult);

namespace server {

/**
 * Anti-replay cache that checks if a unique identifier has been received
 * before.
 */
class ReplayCache {
 public:
  virtual ~ReplayCache() = default;

  virtual folly::SemiFuture<ReplayCacheResult> check(
      std::unique_ptr<folly::IOBuf> identifier) = 0;
};

/**
 * Replay cache implementation that allows replays.
 */
class AllowAllReplayReplayCache : public ReplayCache {
 public:
  ~AllowAllReplayReplayCache() override = default;

  folly::SemiFuture<ReplayCacheResult> check(
      std::unique_ptr<folly::IOBuf>) override {
    return ReplayCacheResult::NotReplay;
  }
};
} // namespace server
} // namespace fizz
