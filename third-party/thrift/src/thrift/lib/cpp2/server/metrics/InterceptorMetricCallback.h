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

  // ============ Stream Metrics ============

  /**
   * Records the completion of a single interceptor's onStreamBegin call.
   */
  virtual void onStreamBeginComplete(
      const ServiceInterceptorQualifiedName& qualifiedName,
      std::chrono::microseconds duration) = 0;

  /**
   * Records the completion of a single interceptor's onStreamPayload call.
   */
  virtual void onStreamPayloadComplete(
      const ServiceInterceptorQualifiedName& qualifiedName,
      std::chrono::microseconds duration) = 0;

  /**
   * Records the completion of a single interceptor's onStreamEnd call.
   */
  virtual void onStreamEndComplete(
      const ServiceInterceptorQualifiedName& qualifiedName,
      std::chrono::microseconds duration) = 0;

  // ============ Total (all-interceptors) Metrics ============

  /**
   * Records the total combined latency of invoking all interceptors'
   * onRequest calls.
   */
  virtual void onRequestTotalComplete(std::chrono::microseconds duration) = 0;

  /**
   * Records the total combined latency of invoking all interceptors'
   * onResponse calls.
   */
  virtual void onResponseTotalComplete(std::chrono::microseconds duration) = 0;

  /**
   * Records the total combined latency of invoking all interceptors'
   * onConnectionAttempted calls.
   */
  virtual void onConnectionAttemptedTotalComplete(
      std::chrono::microseconds duration) = 0;

  /**
   * Records the total combined latency of invoking all interceptors'
   * onConnectionEstablished calls.
   */
  virtual void onConnectionTotalComplete(
      std::chrono::microseconds duration) = 0;

  /**
   * Records the total combined latency of invoking all interceptors'
   * onConnectionClosed calls.
   */
  virtual void onConnectionClosedTotalComplete(
      std::chrono::microseconds duration) = 0;

  /**
   * Records the total combined latency of invoking all interceptors'
   * onStreamBegin calls.
   */
  virtual void onStreamBeginTotalComplete(
      std::chrono::microseconds duration) = 0;

  /**
   * Records the total combined latency of invoking all interceptors'
   * onStreamPayload calls.
   */
  virtual void onStreamPayloadTotalComplete(
      std::chrono::microseconds duration) = 0;

  /**
   * Records the total combined latency of invoking all interceptors'
   * onStreamEnd calls.
   */
  virtual void onStreamEndTotalComplete(std::chrono::microseconds duration) = 0;
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

  void onStreamBeginComplete(
      const ServiceInterceptorQualifiedName&, std::chrono::microseconds) final {
  }

  void onStreamPayloadComplete(
      const ServiceInterceptorQualifiedName&, std::chrono::microseconds) final {
  }

  void onStreamEndComplete(
      const ServiceInterceptorQualifiedName&, std::chrono::microseconds) final {
  }

  void onRequestTotalComplete(std::chrono::microseconds) final {}
  void onResponseTotalComplete(std::chrono::microseconds) final {}
  void onConnectionAttemptedTotalComplete(std::chrono::microseconds) final {}
  void onConnectionTotalComplete(std::chrono::microseconds) final {}
  void onConnectionClosedTotalComplete(std::chrono::microseconds) final {}
  void onStreamBeginTotalComplete(std::chrono::microseconds) final {}
  void onStreamPayloadTotalComplete(std::chrono::microseconds) final {}
  void onStreamEndTotalComplete(std::chrono::microseconds) final {}
};

} // namespace apache::thrift
