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

#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/server/metrics/InterceptorMetricCallback.h>

namespace apache::thrift::detail {

class ThriftServerInternals {
 public:
  ThriftServerInternals(ThriftServer& server) : server_(server) {}

  void setInterceptorMetricCallback(
      std::shared_ptr<InterceptorMetricCallback> interceptorMetricCallback) {
    DCHECK(interceptorMetricCallback);
    server_.interceptorMetricCallback_ = std::move(interceptorMetricCallback);
  }

  void allowDebugInterface(bool value) { server_.allowDebugInterface(value); }

  bool allowDebugInterface() const { return server_.allowDebugInterface(); }

  void allowMonitoringInterface(bool value) {
    server_.allowMonitoringInterface(value);
  }

  bool allowMonitoringInterface() const {
    return server_.allowMonitoringInterface();
  }

  void allowProfilingInterface(bool value) {
    server_.allowProfilingInterface(value);
  }

  bool allowProfilingInterface() const {
    return server_.allowProfilingInterface();
  }

  /**
   * Disable the service health poller. This should be called by Python services
   * before serve() is called if they want to manage their own health status.
   */
  void disableServiceHealthPoller() { server_.disableServiceHealthPoller(); }

  /**
   * Set the service health for Python services.
   * This allows Python services to directly set their health status.
   * The service health poller must already be disabled before calling this
   * method.
   */
  void setServiceHealth(ThriftServer::ServiceHealth health) {
    server_.setServiceHealth(health);
  }

  /**
   * Check if the service health poller has been disabled due to Python services
   * directly setting their health status.
   */
  bool isServiceHealthPollerDisabled() {
    return server_.isServiceHealthPollerDisabled();
  }

 private:
  ThriftServer& server_;
};

} // namespace apache::thrift::detail
