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

#include <folly/io/async/HHWheelTimer.h>
#include <thrift/conformance/stresstest/client/ClientRunnerStats.h>

namespace apache::thrift::stress {

/**
 * Timeout callback that signals test completion by setting a boolean flag.
 * Used to terminate stress tests after a specified duration.
 */
class TestDoneTimeout : public folly::HHWheelTimer::Callback {
 public:
  explicit TestDoneTimeout(bool& testDone) : testDone_(testDone) {}
  void timeoutExpired() noexcept override { testDone_ = true; }

 private:
  bool& testDone_;
};

/**
 * Timeout callback that resets RPC statistics after warmup period.
 * Ensures test measurements start fresh after the warmup phase completes.
 */
class WarmupDoneTimeout : public folly::HHWheelTimer::Callback {
 public:
  explicit WarmupDoneTimeout(ClientRpcStats& stats) : stats_(stats) {}
  void timeoutExpired() noexcept override {
    stats_.numSuccess = 0;
    stats_.numFailure = 0;
    stats_.latencyHistogram.clear();
  }

 private:
  ClientRpcStats& stats_;
};

} // namespace apache::thrift::stress
