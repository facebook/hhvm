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

#include <thrift/conformance/stresstest/client/ClientRunnerStats.h>

namespace apache::thrift::stress {

ClientRpcStats::ClientRpcStats()
    : latencyHistogram(
          /* bucketSize */ 1000,
          /* min */ 0,
          /* max */ 60000000) {}

void ClientRpcStats::combine(const ClientRpcStats& other) {
  latencyHistogram.merge(other.latencyHistogram);
  numSuccess += other.numSuccess;
  numFailure += other.numFailure;
}

void ClientThreadMemoryStats::combine(const ClientThreadMemoryStats& other) {
  threadStart += other.threadStart;
  connectionsEstablished += other.connectionsEstablished;
  p50 = std::max(p50, other.p50);
  p99 = std::max(p99, other.p99);
  p100 = std::max(p100, other.p100);
  connectionsIdle += other.connectionsIdle;
}

} // namespace apache::thrift::stress
