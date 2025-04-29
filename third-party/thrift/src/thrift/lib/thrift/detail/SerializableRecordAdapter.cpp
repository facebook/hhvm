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

#include <thrift/lib/thrift/detail/SerializableRecordAdapter.h>

#include <thrift/lib/thrift/gen-cpp2/record_types.h>

#include <folly/hash/Hash.h>

namespace apache::thrift::dynamic::detail {

bool areByteArraysEqual(
    const type::ByteBuffer& lhs, const type::ByteBuffer& rhs) {
  return folly::IOBufEqualTo{}(lhs, rhs);
}

[[noreturn]] void throwSerializableRecordAccessInactiveKind() {
  folly::throw_exception<std::runtime_error>(
      "tried to access SerializableRecord with inactive kind");
}

std::size_t SerializableRecordHasher::operator()(
    const SerializableRecord& record) const noexcept {
  if (record.empty()) {
    return 0;
  }
  const std::size_t activeMemberHash = record.visit(
      [](auto&& datum) -> std::size_t {
        return std::hash<std::decay_t<decltype(datum)>>{}(datum);
      },
      [](const SerializableRecord::ByteArray& datum) -> std::size_t {
        return folly::IOBufHash{}(*datum);
      },
      [](const SerializableRecord::FieldSet& datum) -> std::size_t {
        return folly::hash::hash_range(
            datum.begin(), datum.end(), datum.size() /* seed */);
      },
      [](const SerializableRecord::List& datum) -> std::size_t {
        return folly::hash::hash_range(
            datum.begin(), datum.end(), datum.size() /* seed */);
      },
      [](const SerializableRecord::Set& datum) -> std::size_t {
        // Hash sets are unordered, so we need a commutative combination
        // function
        return folly::hash::commutative_hash_combine_range_generic(
            datum.size() /* seed */,
            datum.hash_function(),
            datum.begin(),
            datum.end());
      },
      [&](const SerializableRecord::Map& datum) -> std::size_t {
        using EntryHasher = std::hash<SerializableRecord::Map::value_type>;
        // Hash maps are unordered, so we need a commutative combination
        // function
        return folly::hash::commutative_hash_combine_range_generic(
            datum.size() /* seed */, EntryHasher{}, datum.begin(), datum.end());
      });

  using KindHash = std::hash<std::underlying_type_t<SerializableRecord::Kind>>;
  return folly::hash::hash_combine(
      KindHash{}(folly::to_underlying(record.kind())), activeMemberHash);
}

} // namespace apache::thrift::dynamic::detail
