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

#include <thrift/lib/cpp2/dynamic/detail/Datum.h>

#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>

namespace apache::thrift::dynamic {

#if FOLLY_HAVE_MEMORY_RESOURCE

Binary::Binary(const Binary& other) : Binary(other.clone(other.mr_)) {}

Binary& Binary::operator=(const Binary& other) {
  if (this != &other) {
    *this = other.clone(other.mr_);
  }
  return *this;
}

Binary Binary::clone(std::pmr::memory_resource* mr) const {
  if (!data_) {
    return Binary(mr);
  }
  return Binary(data_->clone(mr));
}

#else

Binary::Binary(const Binary& other) : Binary(other.clone()) {}

Binary& Binary::operator=(const Binary& other) {
  if (this != &other) {
    *this = other.clone();
  }
  return *this;
}

Binary Binary::clone(std::pmr::memory_resource* mr) const {
  if (!data_) {
    return Binary(mr);
  }
  return Binary(data_->clone());
}

#endif // FOLLY_HAVE_MEMORY_RESOURCE

folly::io::Cursor Binary::cursor() const {
  return folly::io::Cursor(data_.get());
}

bool operator==(const Binary& lhs, const Binary& rhs) noexcept {
  // If both are null, they're equal
  if (!lhs.data_ && !rhs.data_) {
    return true;
  }
  // If only one is null, they're not equal
  if (!lhs.data_ || !rhs.data_) {
    return false;
  }
  // Compare IOBuf contents
  return folly::IOBufEqualTo()(*lhs.data_, *rhs.data_);
}

} // namespace apache::thrift::dynamic
