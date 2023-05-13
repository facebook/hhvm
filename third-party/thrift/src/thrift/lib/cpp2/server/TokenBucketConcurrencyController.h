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

#include <atomic>
#include <memory>

#include <folly/Executor.h>
#include <folly/TokenBucket.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/logging/xlog.h>

#include <thrift/lib/cpp2/server/ConcurrencyControllerBase.h>
#include <thrift/lib/cpp2/server/RequestPileInterface.h>

namespace apache::thrift {

class TokenBucketConcurrencyController : public ConcurrencyControllerBase {
 public:
  TokenBucketConcurrencyController(
      RequestPileInterface& pile, folly::Executor& executor)
      : pile_(pile),
        executor_(executor),
        innerExecutor_(std::make_unique<folly::CPUThreadPoolExecutor>(1)) {}

  void setExecutionLimitRequests(uint64_t) override {
    XLOG_EVERY_MS(WARNING, 1000)
        << "NaiveTokenBucketConcurrencyController does not support execution limits";
  }
  uint64_t getExecutionLimitRequests() const override {
    XLOG_EVERY_MS(WARNING, 1000)
        << "NaiveTokenBucketConcurrencyController does not support execution limits";
    return 0;
  }

  void setQpsLimit(uint64_t limit) override {
    XLOG_EVERY_MS(INFO, 60 * 1000) << "setQpsLimit: " << limit;
    qpsLimit_.store(limit);
  }

  uint64_t getQpsLimit() const override { return qpsLimit_.load(); }

  void onEnqueued() override;
  void onRequestFinished(ServerRequestData&) override {
    // do nothing
  }

  void stop() override {
    // do nothing
  }

  uint64_t requestCount() const override {
    return 0; // not implemented
  }

  std::string describe() const override {
    return fmt::format(
        "{{TokenBucketConcurrencyController qpsLimit={}}}", qpsLimit_.load());
  }

 private:
  // We already acquired a token, so now we should make as much progress as
  // possible given one token. We will process and require requests from the
  // pile until we find a request that haven't expired yet. We will then process
  // that request and return. To make further progress we need to wait for
  // another token.
  void makeProgress();
  void fastPath();

  static bool expired(const ServerRequest& request);
  static void release(ServerRequest&& request);
  void execute(ServerRequest&& request);

  bool consumeTokens(double tokens) {
    auto qpsLimit = qpsLimit_.load();
    return qpsTokenBucket_.consume(tokens, qpsLimit, qpsLimit);
  }

  bool blockingConsumeTokens(double tokens) {
    auto qpsLimit = qpsLimit_.load();
    return qpsTokenBucket_.consumeWithBorrowAndWait(tokens, qpsLimit, qpsLimit);
  }

  void returnTokens(double tokens) {
    auto qpsLimit = qpsLimit_.load();
    qpsTokenBucket_.returnTokens(tokens, qpsLimit);
  }

  bool consumeToken() { return consumeTokens(1.0); }

  bool blockingConsumeToken() { return blockingConsumeTokens(1.0); }

  void returnToken() { returnTokens(1.0); }

  void clearSlowMode() { slowMode_.store(0); }

  bool isSlowModeEnabled() { return slowMode_.load() == 1; }

  // If slow mode is disabled this method will thread-safely enable it and
  // return true. If slow mode is already enabled it will return false.
  bool enableSlowModeOnce() {
    for (;;) {
      auto old = slowMode_.load();
      if (old == 1) {
        return false;
      }
      if (old == 0) {
        if (slowMode_.compare_exchange_weak(old, 1)) {
          return true;
        }
      }
    }
  }

  RequestPileInterface& pile_;
  folly::Executor& executor_;

  folly::DynamicTokenBucket qpsTokenBucket_;
  folly::relaxed_atomic<uint64_t> qpsLimit_{
      std::numeric_limits<uint64_t>::max()};
  folly::relaxed_atomic<uint16_t> slowMode_{0};
  std::unique_ptr<folly::CPUThreadPoolExecutor> innerExecutor_;
};

} // namespace apache::thrift
