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

#include <cstddef>
#include <thrift/lib/cpp2/server/DecoratorDataStorage.h>

namespace apache::thrift::server {

/**
 * DecoratorDataHandle is used to enable interceptors / service handles to
 * reference data written to by a method decorator.
 */
template <typename T>
class DecoratorDataHandle {
 public:
  static constexpr DecoratorDataHandle uninitialized() {
    return DecoratorDataHandle{0};
  }

  bool isInitialized() const { return ordinal_ != 0; }

 private:
  explicit constexpr DecoratorDataHandle(std::size_t index) : ordinal_{index} {}

  static constexpr DecoratorDataHandle fromIndex(std::size_t index) {
    return DecoratorDataHandle{index + 1};
  }

  constexpr std::size_t getIndex() const { return ordinal_ - 1; }

  // Index is 1 based, 0 is used to represent an invalid handle.
  std::size_t ordinal_;

  friend class DecoratorDataHandleFactory;
  friend class DecoratorData;
};

} // namespace apache::thrift::server
