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

#include <thrift/lib/cpp2/server/metrics/MetricCollector.h>

namespace apache::thrift {

void MetricCollector::setBackend(
    std::shared_ptr<IMetricCollectorBackend> backend) {
  backend_ = std::move(backend);
}

void MetricCollector::requestReceived() const {
  if (backend_) {
    backend_->requestReceived();
  }
}

void MetricCollector::requestRejected(const RequestRejectedScope& scope) const {
  if (backend_) {
    backend_->requestRejected(scope);
  }
}

} // namespace apache::thrift
