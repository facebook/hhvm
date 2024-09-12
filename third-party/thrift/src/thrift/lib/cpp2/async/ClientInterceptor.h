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

 private:
  void internal_onRequest(RequestInfo requestInfo) final {
    if (auto value = onRequest(std::move(requestInfo))) {
      requestInfo.storage->emplace<RequestState>(std::move(*value));
    }
  }

  void internal_onResponse(ResponseInfo responseInfo) final {
    auto* requestState = getValueAsType<RequestState>(*responseInfo.storage);
    onResponse(requestState, std::move(responseInfo));
  }
};

} // namespace apache::thrift
