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

#include <thrift/lib/cpp2/transport/rocket/Types.h>

#include <ostream>

namespace apache {
namespace thrift {
namespace rocket {

void Payload::append(Payload&& other) {
  // append() reflects how data is received on the wire: all (possibly
  // fragmented) metadata arrives first, then the actual data.
  // If we are appending a payload that has metadata, then the current payload
  // should not have data.
  DCHECK(
      !other.hasNonemptyMetadata() ||
      metadataSize_ == buffer_->computeChainDataLength());

  metadataSize_ += other.metadataSize_;
  metadataAndDataSize_ += other.metadataAndDataSize_;
  buffer_->prependChain(std::move(other.buffer_));
}

std::ostream& operator<<(std::ostream& os, StreamId streamId) {
  return os << static_cast<uint32_t>(streamId);
}

} // namespace rocket
} // namespace thrift
} // namespace apache
