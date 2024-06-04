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

#include <thrift/lib/cpp2/server/ServiceInterceptorBase.h>
#include <thrift/lib/cpp2/server/ServiceInterceptorStorage.h>

namespace apache::thrift {

template <class RequestState, class ConnectionState = folly::Unit>
class ServiceInterceptor : public ServiceInterceptorBase {
 public:
  virtual folly::coro::Task<std::optional<RequestState>> onRequest(
      RequestInfo) {
    co_return std::nullopt;
  }
  virtual folly::coro::Task<void> onResponse(RequestState*, ResponseInfo) {
    co_return;
  }

  virtual std::optional<ConnectionState> onConnection(ConnectionInfo) noexcept {
    return std::nullopt;
  }
  virtual void onConnectionClosed(ConnectionState*, ConnectionInfo) noexcept {}

 private:
  void internal_onConnection(ConnectionInfo connectionInfo) noexcept final {
    auto* storage = connectionInfo.storage;
    if (auto value = onConnection(std::move(connectionInfo))) {
      storage->emplace<ConnectionState>(std::move(*value));
    }
  }
  void internal_onConnectionClosed(
      ConnectionInfo connectionInfo) noexcept final {
    ConnectionState* connectionState = connectionInfo.storage->has_value()
        ? std::addressof(
              connectionInfo.storage->value_unchecked<ConnectionState>())
        : nullptr;
    onConnectionClosed(connectionState, std::move(connectionInfo));
  }

  folly::coro::Task<void> internal_onRequest(
      ConnectionInfo, RequestInfo requestInfo) final {
    if (auto value = co_await onRequest(std::move(requestInfo))) {
      requestInfo.storage->emplace<RequestState>(std::move(*value));
    }
  }

  folly::coro::Task<void> internal_onResponse(
      ConnectionInfo, ResponseInfo responseInfo) final {
    RequestState* requestState = responseInfo.storage->has_value()
        ? std::addressof(responseInfo.storage->value_unchecked<RequestState>())
        : nullptr;
    co_await onResponse(requestState, std::move(responseInfo));
  }
};

} // namespace apache::thrift
