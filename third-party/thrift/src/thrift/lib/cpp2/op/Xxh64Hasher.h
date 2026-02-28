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

#include <cstdint>
#include <type_traits>

#include <folly/Hash.h>
#include <folly/Optional.h>
#include <folly/Range.h>
#include <folly/io/IOBuf.h>

#include <xxhash.h>

namespace apache::thrift::op {

class Xxh64Hasher {
 public:
  Xxh64Hasher() : state_{XXH3_createState(), &XXH3_freeState} {
    XXH3_64bits_reset(state_.get());
  }
  XXH64_hash_t getResult() const {
    if (!finalized()) {
      throw std::runtime_error("getResult called on non finalized hasher");
    }
    return *result_;
  }

  template <typename T>
  constexpr std::enable_if_t<std::is_arithmetic_v<T>> combine(const T& val) {
    XXH3_64bits_update(state_.get(), (const unsigned char*)&val, sizeof(val));
  }
  void combine(const folly::IOBuf& value) {
    for (const auto& buf : value) {
      combine(buf);
    }
  }
  void combine(folly::ByteRange value) {
    XXH3_64bits_update(state_.get(), value.data(), value.size());
  }
  void combine(const Xxh64Hasher& other) {
    if (!other.finalized()) {
      throw std::runtime_error("cannot combine non finalized hasher");
    }
    combine(*other.result_);
  }

  void finalize() { result_ = XXH3_64bits_digest(state_.get()); }

  bool operator<(const Xxh64Hasher& other) const {
    if (!finalized()) {
      throw std::runtime_error("less then called on non finalized hasher");
    }
    if (!other.finalized()) {
      throw std::runtime_error("non finalized hasher passed to less then");
    }
    return *result_ < *other.result_;
  }

 private:
  bool finalized() const { return result_.has_value(); }

  folly::Optional<XXH64_hash_t> result_;
  std::unique_ptr<XXH3_state_t, decltype(&XXH3_freeState)> state_;
};

} // namespace apache::thrift::op
