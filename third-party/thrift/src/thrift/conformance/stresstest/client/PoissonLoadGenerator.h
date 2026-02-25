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
#include <chrono>
#include <cmath>
#include <random>
#include <folly/coro/SmallUnboundedQueue.h>
#include <folly/executors/FunctionScheduler.h>
#include <thrift/conformance/stresstest/client/BaseLoadGenerator.h>

namespace apache::thrift::stress {

class PoissonLoadGenerator : public BaseLoadGenerator {
 public:
  explicit PoissonLoadGenerator(
      uint32_t targetQps, std::chrono::duration<int64_t, std::milli> interval)
      : interval_(interval),
        meanRequestsPerBucket_(targetQps * interval_.count() / 1000),
        gen_(std::random_device{}()) {}

  ~PoissonLoadGenerator() override;

  folly::coro::AsyncGenerator<Count> getRequestCount() override;
  void start() override;

 private:
  const std::chrono::duration<int64_t, std::milli> interval_;
  const uint32_t meanRequestsPerBucket_;
  std::atomic<bool> running_{true};
  std::atomic<bool> started_{false};
  folly::coro::SmallUnboundedQueue<Count> queue_;
  std::mt19937_64 gen_{std::random_device()()};
  folly::FunctionScheduler scheduler_;

  void generateRequestSignal();
};

} // namespace apache::thrift::stress
