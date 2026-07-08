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
#include <cstring>
#include <string_view>
#include <type_traits>

#include <folly/io/IOBuf.h>
#include <folly/lang/Bits.h>
#include <folly/ssl/OpenSSLHash.h>

namespace apache::thrift::detail {

template <typename Digest>
class Sha256DigestHasher {
 public:
  Sha256DigestHasher() { digest_.hash_init(EVP_sha256()); }

  void hash(bool v) {
    std::uint8_t byte = v ? 1 : 0;
    digest_.hash_update(folly::ByteRange(&byte, 1));
  }

  template <typename T>
    requires(std::is_integral_v<T> && !std::is_same_v<T, bool>)
  void hash(T v) {
    auto le = folly::Endian::little(v);
    digest_.hash_update(
        folly::ByteRange(
            reinterpret_cast<const std::uint8_t*>(&le), sizeof(le)));
  }

  template <typename T>
    requires std::is_enum_v<T>
  void hash(T v) {
    hash(static_cast<std::int32_t>(v));
  }

  void hash(float v) {
    static_assert(sizeof(float) == 4, "float must be 4 bytes");
    std::uint32_t bits;
    std::memcpy(&bits, &v, sizeof(bits));
    hash(bits);
  }

  void hash(double v) {
    static_assert(sizeof(double) == 8, "double must be 8 bytes");
    std::uint64_t bits;
    std::memcpy(&bits, &v, sizeof(bits));
    hash(bits);
  }

  void hash(std::string_view s) {
    hash(static_cast<std::uint32_t>(s.size()));
    digest_.hash_update(folly::ByteRange(s));
  }

  void hash(folly::ByteRange b) {
    hash(static_cast<std::uint32_t>(b.size()));
    digest_.hash_update(b);
  }

  void hash(const folly::IOBuf& buf) {
    hash(static_cast<std::uint32_t>(buf.computeChainDataLength()));
    digest_.hash_update(buf);
  }

  template <typename OtherDigest>
  void hashDigest(const OtherDigest& digest) {
    digest_.hash_update(
        folly::ByteRange(
            reinterpret_cast<const std::uint8_t*>(digest.data()),
            digest.size()));
  }

  Digest finalize() {
    Digest result;
    digest_.hash_final(
        folly::MutableByteRange(
            reinterpret_cast<std::uint8_t*>(result.data()), result.size()));
    return result;
  }

 private:
  folly::ssl::OpenSSLHash::Digest digest_;
};

} // namespace apache::thrift::detail
