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

#include <thrift/conformance/stresstest/client/PoissonLoadGenerator.h>

namespace apache::thrift::stress {

folly::coro::AsyncGenerator<PoissonLoadGenerator::Count>
PoissonLoadGenerator::getRequestCount() {
  while (running_) {
    auto request = co_await queue_.dequeue();
    co_yield request;
  }
}

void PoissonLoadGenerator::generateRequestSignal() {
  std::poisson_distribution<int32_t> poissonDistribution(
      meanRequestsPerBucket_);
  uint32_t intervalQps = static_cast<int32_t>(poissonDistribution(gen_));
  queue_.enqueue(intervalQps);
}

void PoissonLoadGenerator::start() {
  bool f = false;
  if (started_.compare_exchange_strong(f, true)) {
    auto interval =
        std::chrono::duration_cast<std::chrono::microseconds>(interval_);
    scheduler_.addFunction([&]() { generateRequestSignal(); }, interval);
    scheduler_.start();
  }
}

PoissonLoadGenerator::~PoissonLoadGenerator() {
  running_ = false;
}

} // namespace apache::thrift::stress
