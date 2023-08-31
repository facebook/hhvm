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

#include <optional>
#include <folly/io/async/DelayedDestruction.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/EventBaseLocal.h>

namespace apache::thrift::rocket {

class FlushManager : private folly::EventBase::LoopCallback,
                     public folly::AsyncTimeout {
 public:
  using FlushList = boost::intrusive::list<
      folly::EventBase::LoopCallback,
      boost::intrusive::constant_time_size<false>>;
  explicit FlushManager(folly::EventBase& evb) : evb_(evb) {
    attachEventBase(&evb_);
  }
  static FlushManager& getInstance(folly::EventBase& evb) {
    return getEventBaseLocal().try_emplace(evb, evb);
  }
  void enqueueFlush(folly::EventBase::LoopCallback& writeLoopCallback);
  // has time complexity linear to number of elements in flush list
  size_t getNumPendingClients() const { return flushList_.size(); }

  void timeoutExpired() noexcept override;

  /*
   * When not using setFlushList to manage flushes, this sets the flush
   * policy for the FlushManager. maxPendingFlushes is the number of client
   * flushes which will be batched before scheduling a flush in the next
   * loop callback. maxFlushLatency is the amount of time to wait for
   * maxPendingFlushes before scheduling a loop callback. I.e., it is the
   * latency tolerance for a RocketClient's flush.
   */
  void setFlushPolicy(
      size_t maxPendingFlushes, std::chrono::microseconds maxFlushLatency) {
    flushPolicy_.emplace(maxPendingFlushes, maxFlushLatency);
  }

  /*
   * Reset the flush policy to no policy. Also act as if the timeout elapsed
   * immediately.
   */
  void resetFlushPolicy();

 private:
  void runLoopCallback() noexcept override final;
  static folly::EventBaseLocal<FlushManager>& getEventBaseLocal();

  folly::EventBase& evb_;
  FlushList flushList_;
  bool rescheduled_{false};
  size_t pendingFlushes_{0};
  struct FlushPolicy {
    FlushPolicy(size_t m, std::chrono::microseconds f)
        : maxPendingFlushes(m), maxFlushLatency(f) {}
    size_t maxPendingFlushes{0};
    std::chrono::microseconds maxFlushLatency{0};
  };
  std::optional<FlushPolicy> flushPolicy_;
};

} // namespace apache::thrift::rocket
