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

#include <thrift/lib/cpp2/server/DecoratorDataStorage.h>

namespace apache::thrift::server {

class UntypedDecoratorKey {};

/**
 * DecoratorDataKey<T> is used to enable interceptors / decorators to obtain
 * data handles to the same data.
 *
 * Example usage:
 *
 * constexpr DecoratorDataKey<int> kMyDataKey;
 *
 * class MyMethodDecorator {
 *  public:
 *   void useRequestData(DecoratorDataHandleFactory& handleFactory) {
 *     data_ = handleFactory.get(kMyDataKey);
 *   }
 *  private:
 *   DecoratorDataHandle<int> data_;
 * };
 */
template <typename T>
class DecoratorDataKey : public UntypedDecoratorKey {
 public:
  constexpr DecoratorDataKey() noexcept = default;

  // Not copyable or moveable
  ~DecoratorDataKey() = default;
  DecoratorDataKey(const DecoratorDataKey&) = delete;
  DecoratorDataKey& operator=(const DecoratorDataKey&) = delete;
  DecoratorDataKey(DecoratorDataKey&&) = delete;
  DecoratorDataKey& operator=(DecoratorDataKey&&) = delete;
};

} // namespace apache::thrift::server
