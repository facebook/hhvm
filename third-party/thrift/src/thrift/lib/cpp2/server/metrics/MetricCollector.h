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

#include <memory>

#include <thrift/lib/cpp2/server/metrics/MetricCollectorBackend.h>
#include <thrift/lib/cpp2/server/metrics/Scope.h>

#pragma once

namespace apache::thrift {

/* MetricCollector
 *
 * A wrapper around IMetricCollectorBackend that allows for easier and cleaner
 * instrumentation of various Thrift Server components.
 */
class MetricCollector {
 public:
  MetricCollector() = default;

  explicit MetricCollector(std::shared_ptr<IMetricCollectorBackend> backend)
      : backend_{std::move(backend)} {}

  void setBackend(std::shared_ptr<IMetricCollectorBackend> backend);

  // IMetricCollectorBackend interface wrapper methods
  void requestReceived() const;

  void requestRejected(const RequestRejectedScope&) const;

 private:
  std::shared_ptr<IMetricCollectorBackend> backend_{nullptr};
};

} // namespace apache::thrift
