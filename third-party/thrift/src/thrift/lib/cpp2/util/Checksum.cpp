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

#include <thrift/lib/cpp2/util/Checksum.h>

#include <folly/hash/Checksum.h>

namespace apache::thrift::checksum {

// Calculate crc32c of a IOBuf chain start from skipOffset
uint32_t crc32c(const folly::IOBuf& payload, size_t skipOffset) {
  uint32_t crc32c = ~0U;
  for (auto& buf : payload) {
    if (skipOffset >= buf.size()) {
      skipOffset -= buf.size();
      continue;
    }
    crc32c =
        folly::crc32c(buf.data() + skipOffset, buf.size() - skipOffset, crc32c);
    skipOffset = 0;
  }
  return crc32c;
}

} // namespace apache::thrift::checksum
