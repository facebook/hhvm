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

#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>

#include <memory>
#include <memory_resource>

namespace apache::thrift::dynamic {

/**
 * A Binary type backed by folly::IOBuf for efficient buffer management.
 * Data is exposed using folly::io::Cursor for read access.
 */
class Binary final {
 public:
  // Constructors
  Binary() = default;
  explicit Binary(std::pmr::memory_resource* mr) : mr_(mr) {}
  explicit Binary(
      std::unique_ptr<folly::IOBuf> buf,
      std::pmr::memory_resource* mr = nullptr)
      : data_(std::move(buf)), mr_(mr) {}

  // Copy and move
  Binary(const Binary& other);
  Binary(Binary&&) noexcept = default;
  Binary& operator=(const Binary& other);
  Binary& operator=(Binary&&) noexcept = default;
  ~Binary() = default;

  // Create cursor for reading
  folly::io::Cursor cursor() const;
  // No RWCursor access because it doesn't support memory_resource.

  // Size and empty checks
  size_t computeChainDataLength() const {
    return data_ ? data_->computeChainDataLength() : 0;
  }
  bool empty() const { return !data_ || data_->empty(); }

  // Clone with memory resource
  Binary clone(std::pmr::memory_resource* mr = nullptr) const;

  // Comparison
  friend bool operator==(const Binary& lhs, const Binary& rhs) noexcept;

 private:
  std::unique_ptr<folly::IOBuf> data_;
  std::pmr::memory_resource* mr_ = nullptr;

  template <typename ProtocolWriter>
  friend void serialize(ProtocolWriter& writer, const Binary& binary);
};

} // namespace apache::thrift::dynamic
