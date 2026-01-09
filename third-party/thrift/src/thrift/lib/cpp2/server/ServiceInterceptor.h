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

#include <folly/stop_watch.h>
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

  virtual void onConnectionAttempted(ConnectionInfo) {}

  virtual std::optional<ConnectionState> onConnectionEstablished(
      ConnectionInfo) {
    return std::nullopt;
  }

  virtual void onConnectionClosed(ConnectionState*, ConnectionInfo) noexcept {}

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
    folly::stop_watch<std::chrono::microseconds> onConnectionAttemptTimer;
    onConnectionAttempted(std::move(connectionInfo));
    interceptorMetricCallback.onConnectionAttemptedComplete(
        getQualifiedName(), onConnectionAttemptTimer.elapsed());
  }

  void internal_onConnectionEstablished(
      ConnectionInfo connectionInfo,
      InterceptorMetricCallback& interceptorMetricCallback) final {
    if (isDisabled()) {
      return;
    }
    folly::stop_watch<std::chrono::microseconds> onConnectionTimer;
    auto* storage = connectionInfo.storage;
    if (auto value = onConnectionEstablished(std::move(connectionInfo))) {
      storage->emplace<ConnectionState>(std::move(*value));
    }
    interceptorMetricCallback.onConnectionComplete(
        getQualifiedName(), onConnectionTimer.elapsed());
  }

  void internal_onConnectionClosed(
      ConnectionInfo connectionInfo,
      InterceptorMetricCallback& interceptorMetricCallback) noexcept final {
    if (isDisabled()) {
      return;
    }
    folly::stop_watch<std::chrono::microseconds> onConnectionClosedTimer;
    auto* connectionState =
        getValueAsType<ConnectionState>(*connectionInfo.storage);
    onConnectionClosed(connectionState, std::move(connectionInfo));
    interceptorMetricCallback.onConnectionClosedComplete(
        getQualifiedName(), onConnectionClosedTimer.elapsed());
  }

  folly::coro::Task<void> internal_onRequest(
      ConnectionInfo connectionInfo,
      RequestInfo requestInfo,
      InterceptorMetricCallback& interceptorMetricCallback) final {
    if (isDisabled()) {
      co_return;
    }
    folly::stop_watch<std::chrono::microseconds> onRequestTimer;
    auto* connectionState =
        getValueAsType<ConnectionState>(*connectionInfo.storage);
    auto* storage = requestInfo.storage;
    if (auto value =
            co_await onRequest(connectionState, std::move(requestInfo))) {
      storage->emplace<RequestState>(std::move(*value));
    }
    interceptorMetricCallback.onRequestComplete(
        getQualifiedName(), onRequestTimer.elapsed());
  }

  folly::coro::Task<void> internal_onResponse(
      ConnectionInfo connectionInfo,
      ResponseInfo responseInfo,
      InterceptorMetricCallback& interceptorMetricCallback) final {
    if (isDisabled()) {
      co_return;
    }
    folly::stop_watch<std::chrono::microseconds> onResponseTimer;
    auto* requestState = getValueAsType<RequestState>(*responseInfo.storage);
    auto* connectionState =
        getValueAsType<ConnectionState>(*connectionInfo.storage);
    co_await onResponse(requestState, connectionState, std::move(responseInfo));
    interceptorMetricCallback.onResponseComplete(
        getQualifiedName(), onResponseTimer.elapsed());
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
