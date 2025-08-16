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

#include <stdexcept>
#include <folly/container/span.h>
#include <thrift/lib/cpp2/server/DecoratorDataHandle.h>
#include <thrift/lib/cpp2/server/DecoratorDataStorage.h>

namespace apache::thrift::server {

/**
 * This is the interface that Service Interceptor, Service handlers, and
 * decorators will have access to to read and write from decorator data. put()
 * is only usable from a non-const interface, whereas get() is usable from
 * both const and non-const interfaces.
 */
class DecoratorData {
 public:
  static DecoratorData fromStorage(DecoratorDataStorage& storage);

  ~DecoratorData() = default;
  DecoratorData(const DecoratorData&) = delete;
  DecoratorData& operator=(const DecoratorData&) = delete;
  DecoratorData(DecoratorData&&) = default;
  DecoratorData& operator=(DecoratorData&&) = default;

  template <typename T>
  const T* FOLLY_NULLABLE get(const DecoratorDataHandle<T>& handle) const {
    if (!handle.isInitialized()) {
      throw std::logic_error("Attempted to read from uninitialized handle");
    }

    const auto& entry = decoratorData_[handle.getIndex()];
    if (!entry.has_value()) {
      return nullptr;
    }
    return std::addressof(entry.template value<T>());
  }

  template <typename T, typename... Args>
  void put(const DecoratorDataHandle<T>& handle, Args... args) {
    if (!handle.isInitialized()) {
      throw std::logic_error("Attempted to read from uninitialized handle");
    }
    auto& entry = decoratorData_[handle.getIndex()];
    entry.template emplace<T>(std::forward<Args>(args)...);
  }

 private:
  explicit DecoratorData(DecoratorDataStorage& storage);

  folly::span<DecoratorDataEntry> decoratorData_;
};

} // namespace apache::thrift::server
