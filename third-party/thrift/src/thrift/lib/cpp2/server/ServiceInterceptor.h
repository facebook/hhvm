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

  virtual std::optional<ConnectionState> onConnection(ConnectionInfo) {
    return std::nullopt;
  }
  virtual void onConnectionClosed(ConnectionState*, ConnectionInfo) noexcept {}

  const ServiceInterceptorQualifiedName& getQualifiedName() const final {
    CHECK(qualifiedName_.has_value())
        << "Broken Invariant: ServiceInterceptor::getQualifiedName() was called before ServiceInterceptor::setModuleName()";
    return *qualifiedName_;
  }

 private:
  void internal_onConnection(ConnectionInfo connectionInfo) final {
    auto* storage = connectionInfo.storage;
    if (auto value = onConnection(std::move(connectionInfo))) {
      storage->emplace<ConnectionState>(std::move(*value));
    }
  }
  void internal_onConnectionClosed(
      ConnectionInfo connectionInfo) noexcept final {
    auto* connectionState =
        getValueAsType<ConnectionState>(*connectionInfo.storage);
    onConnectionClosed(connectionState, std::move(connectionInfo));
  }

  folly::coro::Task<void> internal_onRequest(
      ConnectionInfo connectionInfo,
      RequestInfo requestInfo,
      InterceptorMetricCallback& interceptorMetricCallback) final {
    folly::stop_watch<std::chrono::milliseconds> onRequestTimer;
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
      ConnectionInfo connectionInfo, ResponseInfo responseInfo) final {
    auto* requestState = getValueAsType<RequestState>(*responseInfo.storage);
    auto* connectionState =
        getValueAsType<ConnectionState>(*connectionInfo.storage);
    co_await onResponse(requestState, connectionState, std::move(responseInfo));
  }

  void setModuleName(const std::string& moduleName) final {
    CHECK(!qualifiedName_.has_value())
        << "Broken Invariant: ServiceInterceptor::setModuleName() was called more than once";
    qualifiedName_ = {.moduleName = moduleName, .interceptorName = getName()};
  }

  std::optional<ServiceInterceptorQualifiedName> qualifiedName_;
};

} // namespace apache::thrift

#endif // FOLLY_HAS_COROUTINES
