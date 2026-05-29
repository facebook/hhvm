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

#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>

#include <cstdint>
#include <memory>

namespace apache::thrift::fast_thrift::test {

/**
 * Constructs a framed message with a 3-byte big-endian length prefix.
 */
inline std::unique_ptr<folly::IOBuf> constructFrame(
    std::unique_ptr<folly::IOBuf> payload) {
  auto frame = folly::IOBuf::create(
      apache::thrift::fast_thrift::frame::kMetadataLengthSize);
  frame->append(apache::thrift::fast_thrift::frame::kMetadataLengthSize);

  if (payload != nullptr) {
    auto payloadLen = payload->computeChainDataLength();
    uint8_t* data = frame->writableData();
    data[0] = static_cast<uint8_t>((payloadLen >> 16) & 0xFF);
    data[1] = static_cast<uint8_t>((payloadLen >> 8) & 0xFF);
    data[2] = static_cast<uint8_t>(payloadLen & 0xFF);

    frame->appendToChain(std::move(payload));
  }

  return frame;
}

} // namespace apache::thrift::fast_thrift::test
