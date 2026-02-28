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

#include <folly/Function.h>
#include <folly/Preprocessor.h>
#include <thrift/lib/cpp2/Flags.h>

namespace apache::thrift::test {

struct [[nodiscard]] ThriftFlagMockGuard {
  template <typename T, typename V>
  ThriftFlagMockGuard(detail::FlagWrapper<T>& flag, V&& value)
      : unmock_([prevValue = flag.get(), &flag]() mutable {
          flag.setMockValue(std::move(prevValue));
        }) {
    flag.setMockValue(std::forward<V>(value));
  }

  ~ThriftFlagMockGuard() { unmock_(); }

  ThriftFlagMockGuard(const ThriftFlagMockGuard&) = delete;
  ThriftFlagMockGuard& operator=(const ThriftFlagMockGuard&) = delete;
  ThriftFlagMockGuard(ThriftFlagMockGuard&&) = delete;
  ThriftFlagMockGuard& operator=(ThriftFlagMockGuard&&) = delete;

 private:
  folly::Function<void()> unmock_;
};
} // namespace apache::thrift::test

#define THRIFT_FLAG_MOCK_GUARD(_name, _val)                                   \
  ::apache::thrift::test::ThriftFlagMockGuard FB_ANONYMOUS_VARIABLE_ODR_SAFE( \
      THRIFT_FLAG_MOCK_GUARD__##_name)(THRIFT_FLAG_WRAPPER__##_name(), _val)
