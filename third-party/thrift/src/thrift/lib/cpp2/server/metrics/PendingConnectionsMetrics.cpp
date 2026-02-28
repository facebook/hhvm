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

#include <thrift/lib/cpp2/server/metrics/PendingConnectionsMetrics.h>
#include "folly/GLog.h"

THRIFT_FLAG_DEFINE_bool(enable_pending_connections_metrics, true);

namespace {
bool isFeatureDisabled() {
  return !THRIFT_FLAG(enable_pending_connections_metrics);
}
} // namespace

namespace apache::thrift {
PendingConnectionsMetrics::PendingConnectionsMetrics(
    std::shared_ptr<server::TServerObserver> observer)
    : observer_(std::move(observer)) {}

void PendingConnectionsMetrics::onConnectionEnqueuedToIoWorker() {
  if (isFeatureDisabled()) {
    return;
  }
  pendingConnections_++;
  onPendingConnectionsChange();
}

void PendingConnectionsMetrics::onConnectionDropped(
    const std::string& errorMsg) {
  if (isFeatureDisabled()) {
    return;
  }
  // When connection expires in the queue, before it is accepted by an IO
  // Worker, AsyncServerSocker reports the connection as dropped. The queue
  // counter is decremented in this case for accurate queue size tracking.
  if (errorMsg.find("Exceeded deadline for accepting connection") !=
      std::string::npos) {
    pendingConnections_--;
    onPendingConnectionsChange();
  }
}

void PendingConnectionsMetrics::onConnectionDequedByIoWorker() {
  if (isFeatureDisabled()) {
    return;
  }
  pendingConnections_--;
  onPendingConnectionsChange();
}

void PendingConnectionsMetrics::onPendingConnectionsChange() {
  auto pendingConnections = pendingConnections_.load();
  if (pendingConnections >= 0) {
    observer_->pendingConnections(pendingConnections);
  }
}

} // namespace apache::thrift
