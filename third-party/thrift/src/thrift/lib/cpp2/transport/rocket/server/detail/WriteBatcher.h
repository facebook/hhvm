/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <chrono>
#include <memory>
#include <vector>

#include <folly/io/IOBuf.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/HHWheelTimer.h>
#include <folly/net/NetOps.h>

#include <thrift/lib/cpp2/async/MessageChannel.h>
#include <thrift/lib/cpp2/transport/rocket/Types.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketServerConnectionObserver.h>

namespace apache::thrift::rocket {

// Forward declaration to avoid circular dependency
class RocketServerConnection;

// Types that were previously nested in RocketServerConnection
using FdsAndOffsets = std::vector<std::pair<folly::SocketFds, size_t>>;

struct WriteBatchContext {
  // the counts of completed requests in each inflight write
  size_t requestCompleteCount{0};
  // the counts of valid sendCallbacks in each inflight write
  std::vector<apache::thrift::MessageChannel::SendCallbackPtr> sendCallbacks;
  // the WriteEvent objects associated with each write in the batch
  std::vector<RocketServerConnectionObserver::WriteEvent> writeEvents;
  // the raw byte offset at the beginning and end of the inflight write
  RocketServerConnectionObserver::WriteEventBatchContext writeEventsContext;
};

/**
 * WriteBatcher handles batching of writes for performance optimization.
 * It collects multiple writes and flushes them together based on time
 * intervals, count thresholds, or byte size thresholds.
 */
template <typename ConnectionT, template <typename> class ConnectionAdapter>
class WriteBatcher : private folly::EventBase::LoopCallback,
                     private folly::HHWheelTimer::Callback {
  using Connection = ConnectionAdapter<ConnectionT>;

 public:
  WriteBatcher(
      Connection& connection,
      std::chrono::milliseconds batchingInterval,
      size_t batchingSize,
      size_t batchingByteSize)
      : connection_(connection),
        batchingInterval_(batchingInterval),
        batchingSize_(batchingSize),
        batchingByteSize_(batchingByteSize) {}

  void enqueueWrite(
      std::unique_ptr<folly::IOBuf> data,
      apache::thrift::MessageChannel::SendCallbackPtr cb,
      StreamId streamId,
      folly::SocketFds fds) {
    if (cb) {
      cb->sendQueued();
      bufferedWritesContext_.sendCallbacks.push_back(std::move(cb));
    }

    if (auto hasObservers = connection_.numObservers() != 0;
        batchingByteSize_ || hasObservers) {
      auto totalBytesInWrite = data->computeChainDataLength();

      if (hasObservers) {
        bufferedWritesContext_.writeEvents.emplace_back(
            streamId, totalBytesBuffered_, totalBytesInWrite);
      }
      totalBytesBuffered_ += totalBytesInWrite;
    }

    if (!bufferedWrites_) {
      bufferedWrites_ = std::move(data);
      if (batchingInterval_ != std::chrono::milliseconds::zero()) {
        connection_.getEventBase().timer().scheduleTimeout(
            this, batchingInterval_);
      } else {
        connection_.getEventBase().runInLoop(this, true /* thisIteration */);
      }
    } else {
      bufferedWrites_->prependChain(std::move(data));
    }

    // We want the FDs to arrive no later than the last byte of `data`.
    // By attaching the FDs after growing `bufferedWrites_`, it
    // means that `fds` are associated with `[prev offset, offset)`.
    if (!fds.empty()) {
      fdsAndOffsets_.emplace_back(
          std::move(fds),
          // This is costly, but the alternatives are all bad:
          //  - Too fragile: capturing the IOBuf* before `appendToChain`
          //    above would access invalid memory if the chain were coalesced.
          //  - Too messy: `totalBytesBuffered_` as currently implemented
          //    isn't trustworthy -- it's not always set, and even if it
          //    were, it could be wrong because `hasObservers` could've
          //    changed midway through the batch.
          bufferedWrites_->computeChainDataLength());
    }

    ++bufferedWritesCount_;
    if (batchingInterval_ != std::chrono::milliseconds::zero() &&
        (bufferedWritesCount_ == batchingSize_ ||
         (batchingByteSize_ != 0 && totalBytesBuffered_ >= batchingByteSize_ &&
          !earlyFlushRequested_))) {
      earlyFlushRequested_ = true;
      cancelTimeout();
      connection_.getEventBase().runInLoop(this, true /* thisIteration */);
    }
  }

  void enqueueRequestComplete() {
    DCHECK(!empty());
    bufferedWritesContext_.requestCompleteCount++;
  }

  void drain() noexcept {
    if (!bufferedWrites_) {
      return;
    }
    cancelLoopCallback();
    cancelTimeout();
    flushPendingWrites();
  }

  bool empty() const { return !bufferedWrites_; }

 private:
  void runLoopCallback() noexcept final { flushPendingWrites(); }

  void timeoutExpired() noexcept final { flushPendingWrites(); }

  void flushPendingWrites() noexcept {
    bufferedWritesCount_ = 0;
    totalBytesBuffered_ = 0;
    earlyFlushRequested_ = false;
    if (fdsAndOffsets_.empty()) {
      // Fast path: no FDs, write as one batch.
      connection_.flushWrites(
          std::move(bufferedWrites_),
          std::exchange(bufferedWritesContext_, WriteBatchContext{}));
    } else {
      // Slow path: each set of FDs is split into its own batch.
      connection_.flushWritesWithFds(
          std::move(bufferedWrites_),
          std::exchange(bufferedWritesContext_, WriteBatchContext{}),
          std::exchange(fdsAndOffsets_, FdsAndOffsets{}));
    }
  }

  Connection& connection_;
  std::chrono::milliseconds batchingInterval_;
  size_t batchingSize_;
  size_t batchingByteSize_;
  // Callback is scheduled iff bufferedWrites_ is not empty.
  std::unique_ptr<folly::IOBuf> bufferedWrites_;
  size_t bufferedWritesCount_{0};
  size_t totalBytesBuffered_{0};
  bool earlyFlushRequested_{false};
  WriteBatchContext bufferedWritesContext_;
  // Offset in `bufferedWrites_` before which these FDs must be sent.
  FdsAndOffsets fdsAndOffsets_;
};

} // namespace apache::thrift::rocket
