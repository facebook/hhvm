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
#include <folly/ssl/OpenSSLHash.h>

namespace apache {
namespace thrift {
namespace op {

class Sha256Hasher {
 public:
  Sha256Hasher() { hash_.hash_init(EVP_sha256()); }
  std::array<uint8_t, SHA256_DIGEST_LENGTH> getResult() const {
    if (!finalized())
      throw std::runtime_error("getResult called on non finalized hasher");
    return result_.value();
  }

  template <typename T>
  constexpr std::enable_if_t<std::is_arithmetic_v<T>> combine(const T& val) {
    folly::ByteRange r(
        (const unsigned char*)&val, (const unsigned char*)&val + sizeof(val));
    hash_.hash_update(r);
  }
  void combine(const folly::IOBuf& value) {
    for (const auto& buf : value) {
      combine(buf);
    }
  }
  void combine(folly::ByteRange value) { hash_.hash_update(value); }
  void combine(const Sha256Hasher& other) {
    if (!other.finalized())
      throw std::runtime_error("cannot combine non finalized hasher");
    combine(other.result_.value());
  }

  void finalize() {
    result_.emplace();
    folly::MutableByteRange r(result_.value().data(), result_.value().size());
    hash_.hash_final(r);
  }

  bool operator<(const Sha256Hasher& other) const {
    if (!finalized())
      throw std::runtime_error("less then called on non finalized hasher");
    if (!other.finalized())
      throw std::runtime_error("non finalized hasher passed to less then");
    return result_ < other.result_;
  }

 private:
  bool finalized() const { return result_.has_value(); }

  folly::Optional<std::array<uint8_t, SHA256_DIGEST_LENGTH>> result_;
  folly::ssl::OpenSSLHash::Digest hash_;
};

} // namespace op
} // namespace thrift
} // namespace apache
