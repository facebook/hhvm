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

#include <thrift/lib/cpp/server/TServerObserver.h>
#include <thrift/lib/cpp2/Flags.h>

namespace apache::thrift {
// This class is responsible for recording metrics associated with inflight
// connections. An inflight connection is a connection that has been accepted by
// an Acceptor thread, but has not yet been accepted by an IO Worker thread for
// further processing of heavier weight operations like TLS Handshake.
class PendingConnectionsMetrics {
 public:
  explicit PendingConnectionsMetrics(
      std::shared_ptr<server::TServerObserver> observer);

  void onConnectionEnqueuedToIoWorker();
  void onConnectionDequedByIoWorker();
  void onConnectionDropped(const std::string& errorMsg);

 private:
  std::shared_ptr<server::TServerObserver> observer_;
  folly::relaxed_atomic<int32_t> pendingConnections_{0};

  void onPendingConnectionsChange();
};
} // namespace apache::thrift
