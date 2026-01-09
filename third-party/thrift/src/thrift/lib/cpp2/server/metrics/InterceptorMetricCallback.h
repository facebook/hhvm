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

#include <chrono>
#include <thrift/lib/cpp2/server/ServiceInterceptorQualifiedName.h>

namespace apache::thrift {

class InterceptorMetricCallback {
 public:
  virtual ~InterceptorMetricCallback() = default;

  /**
   * Records the completion of a single interceptor's onRequest call.
   *
   * @param qualifiedName The fully qualified name of the interceptor.
   * @param onRequestDuration The time it took to complete the onResponse call.
   */
  virtual void onRequestComplete(
      const ServiceInterceptorQualifiedName& qualifiedName,
      std::chrono::microseconds onRequestDuration) = 0;

  /**
   * Records the completion of a single interceptor's onResponse call.
   *
   * @param qualifiedName The fully qualified name of the interceptor.
   * @param onResponseDuration The time it took to complete the onResponse call.
   */
  virtual void onResponseComplete(
      const ServiceInterceptorQualifiedName& qualifiedName,
      std::chrono::microseconds onResponseDuration) = 0;

  /**
   * Records the completion of a single interceptor's onConnectionAttempted
   * call.
   *
   * @param qualifiedName The fully qualified name of the interceptor.
   * @param onConnectionAttemptDuration The time it took to complete the
   * onConnectionAttempted call.
   */
  virtual void onConnectionAttemptedComplete(
      const ServiceInterceptorQualifiedName& qualifiedName,
      std::chrono::microseconds onConnectionAttemptDuration) = 0;

  /**
   * Records the completion of a single interceptor's onConnectionEstablished
   * call.
   *
   * @param qualifiedName The fully qualified name of the interceptor.
   * @param onConnectionDuration The time it took to complete the
   * onConnectionEstablished call.
   */
  virtual void onConnectionComplete(
      const ServiceInterceptorQualifiedName& qualifiedName,
      std::chrono::microseconds onConnectionDuration) = 0;

  /**
   * Records the completion of a single interceptor's onResponse call.
   *
   * @param qualifiedName The fully qualified name of the interceptor.
   * @param onConnectionClosedDuration The time it took to complete the
   * onResponse call.
   */
  virtual void onConnectionClosedComplete(
      const ServiceInterceptorQualifiedName& qualifiedName,
      std::chrono::microseconds onConnectionClosedDuration) = 0;
};

class NoopInterceptorMetricCallback : public InterceptorMetricCallback {
 public:
  void onRequestComplete(
      const ServiceInterceptorQualifiedName&, std::chrono::microseconds) final {
  }

  void onResponseComplete(
      const ServiceInterceptorQualifiedName&, std::chrono::microseconds) final {
  }

  void onConnectionAttemptedComplete(
      const ServiceInterceptorQualifiedName&, std::chrono::microseconds) final {
  }

  void onConnectionComplete(
      const ServiceInterceptorQualifiedName&, std::chrono::microseconds) final {
  }

  void onConnectionClosedComplete(
      const ServiceInterceptorQualifiedName&, std::chrono::microseconds) final {
  }
};

} // namespace apache::thrift
