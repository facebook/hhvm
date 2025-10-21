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
#include <folly/io/IOBufQueue.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/HHWheelTimer.h>
#include <folly/net/NetOps.h>

#include <thrift/lib/cpp2/async/MessageChannel.h>
#include <thrift/lib/cpp2/transport/rocket/Types.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketServerConnectionObserver.h>
#include <thrift/lib/cpp2/transport/rocket/server/detail/WriteBatchTypes.h>

namespace apache::thrift::rocket {

// Forward declaration to avoid circular dependency
class RocketServerConnection;

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

    // PERFORMANCE: Calculate byte size only once (was O(n)
    // computeChainDataLength)
    size_t totalBytesInWrite = 0;
    if (auto hasObservers = connection_.numObservers() != 0;
        batchingByteSize_ || hasObservers) {
      totalBytesInWrite = data ? data->computeChainDataLength() : 0;

      if (hasObservers) {
        bufferedWritesContext_.writeEvents.emplace_back(
            streamId, totalBytesBuffered_, totalBytesInWrite);
      }
      totalBytesBuffered_ += totalBytesInWrite;
    }

    // PERFORMANCE: Use IOBufQueue for O(1) append instead of O(n) prependChain
    bool wasEmpty = bufferedWritesQueue_.empty();
    if (data) {
      bufferedWritesQueue_.append(std::move(data));
    }

    if (wasEmpty && !bufferedWritesQueue_.empty()) {
      if (batchingInterval_ != std::chrono::milliseconds::zero()) {
        connection_.getEventBase().timer().scheduleTimeout(
            this, batchingInterval_);
      } else {
        connection_.getEventBase().runInLoop(this, true /* thisIteration */);
      }
    }

    // We want the FDs to arrive no later than the last byte of `data`.
    // By attaching the FDs after growing the buffer, FDs are associated
    // with `[prev offset, current offset)`.
    if (!fds.empty()) {
      fdsAndOffsets_.emplace_back(
          std::move(fds),
          // PERFORMANCE: Use O(1) chainLength() instead of O(n)
          // computeChainDataLength() IOBufQueue maintains the chain length
          // incrementally
          bufferedWritesQueue_.chainLength());
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
    if (bufferedWritesQueue_.empty()) {
      return;
    }
    cancelLoopCallback();
    cancelTimeout();
    flushPendingWrites();
  }

  bool empty() const { return bufferedWritesQueue_.empty(); }

 private:
  void runLoopCallback() noexcept final { flushPendingWrites(); }

  void timeoutExpired() noexcept final { flushPendingWrites(); }

  void flushPendingWrites() noexcept {
    bufferedWritesCount_ = 0;
    totalBytesBuffered_ = 0;
    earlyFlushRequested_ = false;

    // PERFORMANCE: Extract buffered data from IOBufQueue (O(1) operation)
    auto bufferedWrites = bufferedWritesQueue_.move();

    if (fdsAndOffsets_.empty()) {
      // Fast path: no FDs, write as one batch.
      connection_.flushWrites(
          std::move(bufferedWrites),
          std::exchange(
              bufferedWritesContext_,
              apache::thrift::rocket::WriteBatchContext{}));
    } else {
      // Slow path: each set of FDs is split into its own batch.
      connection_.flushWritesWithFds(
          std::move(bufferedWrites),
          std::exchange(
              bufferedWritesContext_,
              apache::thrift::rocket::WriteBatchContext{}),
          std::exchange(
              fdsAndOffsets_, apache::thrift::rocket::FdsAndOffsets{}));
    }
  }

  Connection& connection_;
  std::chrono::milliseconds batchingInterval_;
  size_t batchingSize_;
  size_t batchingByteSize_;
  // PERFORMANCE: Use IOBufQueue for O(1) append operations instead of O(n)
  // chain manipulation
  folly::IOBufQueue bufferedWritesQueue_;
  size_t bufferedWritesCount_{0};
  size_t totalBytesBuffered_{0};
  bool earlyFlushRequested_{false};
  apache::thrift::rocket::WriteBatchContext bufferedWritesContext_;
  // Offset in buffered writes before which these FDs must be sent.
  apache::thrift::rocket::FdsAndOffsets fdsAndOffsets_;
};

} // namespace apache::thrift::rocket
