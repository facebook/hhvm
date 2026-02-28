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

#include <thrift/lib/cpp2/protocol/detail/index.h>

#include <cassert>
#include <limits>
#include <memory>

#include <xxhash.h>

namespace apache::thrift::detail {

void Xxh3Hasher::init() {
  state = static_cast<void*>(XXH3_createState());
  XXH3_64bits_reset(static_cast<XXH3_state_t*>(state));
}

Xxh3Hasher::~Xxh3Hasher() {
  if (is_initialized()) {
    XXH3_freeState(static_cast<XXH3_state_t*>(state));
  }
}

void Xxh3Hasher::update(folly::io::Cursor cursor) {
  assert(is_initialized());
  while (!cursor.isAtEnd()) {
    const auto buf = cursor.peekBytes();
    XXH3_64bits_update(
        static_cast<XXH3_state_t*>(state), buf.data(), buf.size());
    cursor += buf.size();
  }
}

Xxh3Hasher::operator int64_t() {
  assert(is_initialized());
  return XXH3_64bits_digest(static_cast<XXH3_state_t*>(state));
}

void throwChecksumMismatch(int64_t expected, int64_t actual) {
  gLazyDeserializationIsDisabledDueToChecksumMismatch = true;
  throw TProtocolException(
      TProtocolException::CHECKSUM_MISMATCH,
      fmt::format("expected ({}) != actual ({})", expected, actual));
}

} // namespace apache::thrift::detail
