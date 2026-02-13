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

#include <thrift/lib/cpp2/async/ClientInterceptorBase.h>
#include <thrift/lib/cpp2/async/ClientInterceptorStorage.h>

#include <optional>

namespace apache::thrift {

/**
 * Base class for client interceptors with typed request state.
 *
 * RequestState is created in onRequest and persists through onResponse and
 * all stream callbacks (onStreamBegin, onStreamPayload, onStreamEnd). This
 * allows interceptors to maintain state across the entire RPC lifecycle.
 */
template <class RequestState>
class ClientInterceptor : public ClientInterceptorBase {
 private:
  template <class T, std::size_t Size, std::size_t Align>
  static T* getValueAsType(
      apache::thrift::util::TypeErasedValue<Size, Align>& storage) {
    return storage.has_value()
        ? std::addressof(storage.template value_unchecked<T>())
        : nullptr;
  }

 public:
  virtual std::optional<RequestState> onRequest(RequestInfo) {
    return std::nullopt;
  }
  virtual void onResponse(RequestState*, ResponseInfo) {}

  // Stream lifecycle hooks - override in derived classes to handle streams.
  // RequestState created in onRequest is available in all stream callbacks.
  virtual void onStreamBegin(RequestState*) {}
  virtual void onStreamPayload(RequestState*, StreamPayloadInfo) {}
  virtual void onStreamEnd(RequestState*, const StreamEndInfo&) noexcept {}

 private:
  void internal_onRequest(RequestInfo requestInfo) final {
    auto* storage = requestInfo.storage;
    if (auto value = onRequest(std::move(requestInfo))) {
      storage->emplace<RequestState>(std::move(*value));
    }
  }

  void internal_onResponse(ResponseInfo responseInfo) final {
    auto* requestState = getValueAsType<RequestState>(*responseInfo.storage);
    onResponse(requestState, std::move(responseInfo));
  }

  void internal_onStreamBegin(
      detail::ClientInterceptorOnRequestStorage* storage) final {
    auto* requestState = getValueAsType<RequestState>(*storage);
    onStreamBegin(requestState);
  }

  void internal_onStreamPayload(
      detail::ClientInterceptorOnRequestStorage* storage,
      StreamPayloadInfo payloadInfo) final {
    auto* requestState = getValueAsType<RequestState>(*storage);
    onStreamPayload(requestState, std::move(payloadInfo));
  }

  void internal_onStreamEnd(
      detail::ClientInterceptorOnRequestStorage* storage,
      const StreamEndInfo& endInfo) noexcept final {
    auto* requestState = getValueAsType<RequestState>(*storage);
    onStreamEnd(requestState, endInfo);
  }
};

} // namespace apache::thrift
