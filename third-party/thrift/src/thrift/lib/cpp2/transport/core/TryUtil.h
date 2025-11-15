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

#include <utility>
#include <folly/Portability.h>
#include <folly/Try.h>

namespace apache::thrift {

template <typename T>
[[nodiscard]] folly::Try<T> collapseTry(
    folly::Try<folly::Try<T>>&& arg) noexcept {
  static_assert(
      std::is_nothrow_move_constructible_v<folly::Try<T>>,
      "move constructor should not throw");
  if (arg.hasException()) {
    return folly::Try<T>(std::move(arg.exception()));
  } else if (arg.hasValue()) {
    return std::move(arg.value());
  } else {
    return {};
  }
}

} // namespace apache::thrift
