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

#include <concepts>
#include <string_view>
#include <folly/coro/Task.h>
#include <folly/io/IOBuf.h>

namespace apache::thrift::fast_thrift::channel_pipeline {

// Handler ID type - used for efficient lookups
using HandlerId = uint64_t;

// Compile-time FNV-1a hash for handler identification
constexpr HandlerId fnv1a_hash(std::string_view sv) {
  HandlerId hash = 14695981039346656037ULL;
  for (char c : sv) {
    hash ^= static_cast<HandlerId>(c);
    hash *= 1099511628211ULL;
  }
  return hash;
}

// User-defined literal for compile-time handler IDs
constexpr HandlerId operator""_hid(const char* str, size_t len) {
  return fnv1a_hash({str, len});
}

template <typename T>
concept MoveOnly = std::movable<T> && !std::copyable<T>;

// Result type for all data-path operations.
// [[nodiscard]] ensures callers handle backpressure and errors.
enum class [[nodiscard]] Result { Success, Backpressure, Error };

template <typename T>
using Task = folly::coro::Task<T>;

// Direction of the head-to-tail data path.
// Controls how reads and writes are routed through the handler chain.
//   Write (default): reads flow 0→N→head, writes flow N→0→tail
//   Read:            reads flow N→0→tail, writes flow 0→N→head
enum class HeadToTailOp { Read, Write };

} // namespace apache::thrift::fast_thrift::channel_pipeline

// Include TypeErasedBox for backward compatibility
// TypeErasedBox is now in its own file per C++ guidelines
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
