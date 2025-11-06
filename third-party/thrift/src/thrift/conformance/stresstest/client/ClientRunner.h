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

#include <folly/coro/AsyncScope.h>
#include <folly/coro/Task.h>
#include <folly/io/async/HHWheelTimer.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/synchronization/RelaxedAtomic.h>
#include <thrift/conformance/stresstest/client/ClientConfig.h>
#include <thrift/conformance/stresstest/client/ClientFactory.h>
#include <thrift/conformance/stresstest/client/PoissonLoadGenerator.h>
#include <thrift/conformance/stresstest/client/StressTestBase.h>
#include <thrift/conformance/stresstest/if/gen-cpp2/StressTest.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Frames.h>

namespace apache::thrift::stress {

class ClientThread;

struct ClientThreadMemoryStats {
  void combine(const ClientThreadMemoryStats& other);
  size_t threadStart{0};
  size_t connectionsEstablished{0};
  size_t p50{0};
  size_t p99{0};
  size_t p100{0};
  size_t connectionsIdle{0};
};

class ClientRunner {
 public:
  explicit ClientRunner(const ClientConfig& config);
  ~ClientRunner();

  void run(const StressTestBase* test);

  ClientRpcStats getRpcStats() const;
  ClientThreadMemoryStats getMemoryStats() const;

  void resetStats();

 private:
  bool started_{false};
  bool stopped_{false};
  bool continuous_{false};
  bool useLoadGenerator_;
  std::vector<std::unique_ptr<PoissonLoadGenerator>> loadGenerator_;
  std::vector<std::unique_ptr<ClientThread>> clientThreads_;
};

} // namespace apache::thrift::stress
