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

#include <thrift/lib/cpp2/reflection/internal/legacy_reflection_type_id.h>

#include <folly/lang/Bits.h>
#include <folly/ssl/OpenSSLHash.h>

namespace apache::thrift::legacy_reflection_detail {

id_t get_type_id(reflection::Type type, folly::StringPiece name) {
  union {
    id_t val;
    std::uint8_t buf[20];
  } hash;
  folly::ssl::OpenSSLHash::sha1(folly::range(hash.buf), name);
  const auto truncated = folly::Endian::little(hash.val);
  constexpr auto kTypeBits = 5;
  constexpr auto kTypeMask = (id_t(1) << kTypeBits) - 1;
  return (truncated & ~kTypeMask) | id_t(type);
}

} // namespace apache::thrift::legacy_reflection_detail
