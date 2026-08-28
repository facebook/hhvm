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

#include <cstdint>
#include <memory>
#include <vector>

#include <folly/CPortability.h>
#include <folly/Executor.h>
#include <folly/Synchronized.h>
#include <folly/io/async/AsyncTimeout.h>
#include <folly/io/async/EventBase.h>
#include <thrift/conformance/stresstest/if/gen-cpp2/StressTestResult_types.h>

namespace apache::thrift::stress {

class StressTestServerStats {
 public:
  void init(
      std::vector<folly::Executor::KeepAlive<folly::EventBase>>& evbs,
      uint32_t intervalSeconds);
  void collectFinal();
  void recordConnection(ConnectionMode mode);
  ServerResult publish(ResultMetadata metadata) const;

 private:
  class IoUringStatsTimer;
  struct IoUringStatsTimerRegistration {
    folly::EventBase* eventBase;
    std::shared_ptr<IoUringStatsTimer> timer;
  };

  struct ConnectionState {
    int64_t unknownConnections{0};
    int64_t plaintextConnections{0};
    int64_t tlsConnections{0};
    int64_t stopTlsV2Connections{0};
  };

  folly::Synchronized<ConnectionState> connectionState_;
  ZcrxCounters zcrxCounters_;
  std::vector<IoUringStatsTimerRegistration> timers_;
};

} // namespace apache::thrift::stress
