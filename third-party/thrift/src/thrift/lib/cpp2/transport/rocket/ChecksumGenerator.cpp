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

#include <folly/ThreadLocal.h>
#include <folly/hash/Checksum.h>
#include <folly/io/Cursor.h>
#include <thrift/lib/cpp2/transport/rocket/ChecksumGenerator.h>

namespace apache::thrift::rocket::detail {

struct Destory {
  void operator()(XXH3_state_t* state) {
    if (state != nullptr) {
      XXH3_freeState(state);
    }
  }
};

using XXH3StatePtr = std::unique_ptr<XXH3_state_t, Destory>;

static folly::ThreadLocal<XXH3StatePtr> xxh3State_ =
    folly::ThreadLocal<XXH3StatePtr>(
        []() { return XXH3StatePtr(XXH3_createState(), Destory{}); });

XXH3_state_t* XXH3Logic::getXXH3State() {
  return xxh3State_->get();
}

int64_t XXH3Logic::xxh3(folly::IOBuf& buffer, int64_t salt) {
  XXH3_state_t* const state = getXXH3State();
  if (XXH3_64bits_reset_withSeed(state, salt) == XXH_ERROR) {
    throw std::runtime_error("XXH64_reset failed");
  }

  folly::io::Cursor cursor(&buffer);
  while (!cursor.isAtEnd()) {
    auto bytes = cursor.peekBytes();
    auto ret = XXH3_64bits_update(state, bytes.data(), bytes.size());
    if (ret == XXH_ERROR) {
      throw std::runtime_error("XXH64_update failed");
    }
    cursor.skip(bytes.size());
  }

  auto ret = XXH3_64bits_digest(state);
  if (ret == XXH_ERROR) {
    throw std::runtime_error("XXH64_digest failed");
  }
  return ret;
}

int64_t CRC32Logic::crc32c(folly::IOBuf& buffer, int64_t salt) {
  auto checksum = salt;
  size_t count = 0;
  folly::io::Cursor cursor(&buffer);
  while (!cursor.isAtEnd()) {
    ++count;
    auto bytes = cursor.peekBytes();
    checksum = folly::crc32c(bytes.data(), bytes.size(), checksum);
    cursor.skip(bytes.size());
  }
  FOLLY_SAFE_DCHECK(count == buffer.countChainElements());
  return checksum;
}

} // namespace apache::thrift::rocket::detail
