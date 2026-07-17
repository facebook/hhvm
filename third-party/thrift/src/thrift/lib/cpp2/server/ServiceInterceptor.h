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

#include <folly/coro/Nothrow.h>
#include <folly/coro/Task.h>
#include <thrift/lib/cpp2/server/ServiceInterceptorBase.h>
#include <thrift/lib/cpp2/server/ServiceInterceptorControl.h>
#include <thrift/lib/cpp2/server/ServiceInterceptorStorage.h>

#if FOLLY_HAS_COROUTINES

namespace apache::thrift {

template <class RequestState, class ConnectionState = folly::Unit>
class ServiceInterceptor : public ServiceInterceptorBase {
 private:
  template <class T, std::size_t Size, std::size_t Align>
  static T* getValueAsType(
      apache::thrift::util::TypeErasedValue<Size, Align>& storage) {
    return storage.has_value()
        ? std::addressof(storage.template value_unchecked<T>())
        : nullptr;
  }

 public:
  virtual folly::coro::Task<std::optional<RequestState>> onRequest(
      ConnectionState*, RequestInfo) {
    co_return std::nullopt;
  }
  virtual folly::coro::Task<void> onResponse(
      RequestState*, ConnectionState*, ResponseInfo) {
    co_return;
  }

  virtual void onRequestReceived(ReceivedRequestInfo) {}

  virtual void onRequestDropped(DroppedRequestInfo) {}

  virtual void onConnectionAttempted(ConnectionInfo) {}

  virtual std::optional<ConnectionState> onConnectionEstablished(
      ConnectionInfo) {
    return std::nullopt;
  }

  virtual void onConnectionClosed(ConnectionState*, ConnectionInfo) noexcept {}

  // ============ Streaming Methods ============

  /**
   * Override to handle stream begin event.
   * Use RequestState for any state needed during streaming.
   *
   * NOTE: ConnectionState is not available during stream callbacks because
   * streams may outlive the connection. If you need connection-level state
   * during streaming, capture it in RequestState during onRequest.
   */
  virtual std::optional<folly::coro::Task<void>> onStreamBegin(
      RequestState*, StreamInfo) {
    return std::nullopt;
  }

  /**
   * Override to process each streaming payload.
   * Called BEFORE serialization with access to typed payload.
   *
   * @param requestState Per-request state from onRequest
   * @param info Payload information including typed payload reference
   */
  virtual std::optional<folly::coro::Task<void>> onStreamPayload(
      RequestState*, StreamPayloadInfo) {
    return std::nullopt;
  }

  /**
   * Override to handle stream completion/error/cancellation.
   * Clean up any per-stream resources here.
   *
   * @param requestState Per-request state from onRequest
   * @param info Stream end information
   */
  virtual std::optional<folly::coro::Task<void>> onStreamEnd(
      RequestState*, const StreamEndInfo&) {
    return std::nullopt;
  }

  const ServiceInterceptorQualifiedName& getQualifiedName() const final {
    return qualifiedName_;
  }

 private:
  void internal_onConnectionAttempted(
      ConnectionInfo connectionInfo,
      InterceptorMetricCallback& interceptorMetricCallback) final {
    if (isDisabled()) {
      return;
    }
    InterceptorTimer timer(interceptorMetricCallback);
    onConnectionAttempted(std::move(connectionInfo));
    interceptorMetricCallback.onConnectionAttemptedComplete(
        getQualifiedName(), timer.elapsed());
  }

  void internal_onConnectionEstablished(
      ConnectionInfo connectionInfo,
      InterceptorMetricCallback& interceptorMetricCallback) final {
    if (isDisabled()) {
      return;
    }
    InterceptorTimer timer(interceptorMetricCallback);
    auto* storage = connectionInfo.storage;
    if (auto value = onConnectionEstablished(std::move(connectionInfo))) {
      storage->emplace<ConnectionState>(std::move(*value));
    }
    interceptorMetricCallback.onConnectionComplete(
        getQualifiedName(), timer.elapsed());
  }

  void internal_onConnectionClosed(
      ConnectionInfo connectionInfo,
      InterceptorMetricCallback& interceptorMetricCallback) noexcept final {
    if (isDisabled()) {
      return;
    }
    InterceptorTimer timer(interceptorMetricCallback);
    auto* connectionState =
        getValueAsType<ConnectionState>(*connectionInfo.storage);
    onConnectionClosed(connectionState, std::move(connectionInfo));
    interceptorMetricCallback.onConnectionClosedComplete(
        getQualifiedName(), timer.elapsed());
  }

  void internal_onRequestReceived(
      ReceivedRequestInfo receivedRequestInfo) final {
    if (isDisabled()) {
      return;
    }
    onRequestReceived(receivedRequestInfo);
  }

  void internal_onRequestDropped(DroppedRequestInfo droppedRequestInfo) final {
    if (isDisabled()) {
      return;
    }
    onRequestDropped(droppedRequestInfo);
  }

  folly::coro::Task<void> internal_onRequest(
      ConnectionInfo connectionInfo,
      RequestInfo requestInfo,
      InterceptorMetricCallback& interceptorMetricCallback) final {
    if (isDisabled()) {
      co_return;
    }
    InterceptorTimer timer(interceptorMetricCallback);
    auto* connectionState =
        getValueAsType<ConnectionState>(*connectionInfo.storage);
    auto* storage = requestInfo.storage;
    auto value = co_await folly::coro::co_nothrow(
        onRequest(connectionState, std::move(requestInfo)));
    if (value) {
      storage->emplace<RequestState>(std::move(*value));
    }
    interceptorMetricCallback.onRequestComplete(
        getQualifiedName(), timer.elapsed());
  }

  folly::coro::Task<void> internal_onResponse(
      ConnectionInfo connectionInfo,
      ResponseInfo responseInfo,
      InterceptorMetricCallback& interceptorMetricCallback) final {
    if (isDisabled()) {
      co_return;
    }
    InterceptorTimer timer(interceptorMetricCallback);
    auto* requestState = getValueAsType<RequestState>(*responseInfo.storage);
    auto* connectionState =
        getValueAsType<ConnectionState>(*connectionInfo.storage);
    co_await onResponse(requestState, connectionState, std::move(responseInfo));
    interceptorMetricCallback.onResponseComplete(
        getQualifiedName(), timer.elapsed());
  }

  // ============ Internal Stream Implementations ============

  folly::coro::Task<void> internal_onStreamBegin(
      StreamInfo streamInfo,
      InterceptorMetricCallback& interceptorMetricCallback) final {
    if (isDisabled()) {
      co_return;
    }
    InterceptorTimer timer(interceptorMetricCallback);
    auto* requestState =
        getValueAsType<RequestState>(*streamInfo.requestStorage);
    auto maybeTask = onStreamBegin(requestState, std::move(streamInfo));
    if (maybeTask) {
      co_await std::move(*maybeTask);
    }
    interceptorMetricCallback.onStreamBeginComplete(
        getQualifiedName(), timer.elapsed());
  }

  std::optional<folly::coro::Task<void>> internal_onStreamPayload(
      StreamPayloadInfo payloadInfo) final {
    if (isDisabled()) {
      return std::nullopt;
    }
    auto* requestState =
        getValueAsType<RequestState>(*payloadInfo.requestStorage);
    return onStreamPayload(requestState, std::move(payloadInfo));
  }

  folly::coro::Task<void> internal_onStreamEnd(
      StreamEndInfo endInfo,
      InterceptorMetricCallback& interceptorMetricCallback) final {
    if (isDisabled()) {
      co_return;
    }
    InterceptorTimer timer(interceptorMetricCallback);
    auto* requestState = getValueAsType<RequestState>(*endInfo.requestStorage);
    auto maybeTask = onStreamEnd(requestState, endInfo);
    if (maybeTask) {
      co_await std::move(*maybeTask);
    }
    interceptorMetricCallback.onStreamEndComplete(
        getQualifiedName(), timer.elapsed());
  }

  void setModuleName(const std::string& moduleName) final {
    qualifiedName_.setName(moduleName, getName());
  }

  bool isDisabled() const { return control_.isDisabled(); }

  ServiceInterceptorQualifiedName qualifiedName_;
  ServiceInterceptorControl control_{qualifiedName_};
};

} // namespace apache::thrift

#endif // FOLLY_HAS_COROUTINES
