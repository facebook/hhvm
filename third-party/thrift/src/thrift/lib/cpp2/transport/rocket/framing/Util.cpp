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

#include <thrift/lib/cpp2/transport/rocket/framing/Util.h>

#include <cstdint>
#include <exception>
#include <string>

#include <folly/Conv.h>

namespace apache {
namespace thrift {
namespace rocket {
namespace detail {

[[noreturn]] void throwUnexpectedFrameType(uint8_t frameType) {
  throw std::runtime_error(
      folly::to<std::string>("Parsed unexpected frame type: ", frameType));
}

} // namespace detail

StreamId readStreamId(folly::io::Cursor& cursor) {
  return StreamId{cursor.readBE<StreamId::underlying_type>()};
}

size_t readFrameOrMetadataSize(folly::io::Cursor& cursor) {
  std::array<uint8_t, 3> bytes;
  cursor.pull(bytes.data(), bytes.size());

  return readFrameOrMetadataSize(bytes);
}

// Rsocket frame is 24-bits (3 bytes)
size_t readFrameOrMetadataSize(std::array<uint8_t, 3> bytes) {
  return (static_cast<size_t>(bytes[0]) << 16) |
      (static_cast<size_t>(bytes[1]) << 8) | static_cast<size_t>(bytes[2]);
}

std::pair<uint8_t, Flags> readFrameTypeAndFlagsUnsafe(
    folly::io::Cursor& cursor) {
  const uint16_t frameTypeAndFlags = cursor.readBE<uint16_t>();
  const uint8_t frameType = frameTypeAndFlags >> Flags::kBits;
  const Flags flags(frameTypeAndFlags & Flags::kMask);
  return {frameType, flags};
}

std::pair<FrameType, Flags> readFrameTypeAndFlags(folly::io::Cursor& cursor) {
  const auto pair = readFrameTypeAndFlagsUnsafe(cursor);
  switch (static_cast<FrameType>(pair.first)) {
    case FrameType::SETUP:
    case FrameType::REQUEST_RESPONSE:
    case FrameType::REQUEST_FNF:
    case FrameType::REQUEST_STREAM:
    case FrameType::REQUEST_CHANNEL:
    case FrameType::REQUEST_N:
    case FrameType::CANCEL:
    case FrameType::PAYLOAD:
    case FrameType::ERROR:
    case FrameType::METADATA_PUSH:
    case FrameType::KEEPALIVE:
    case FrameType::EXT:
      return {static_cast<FrameType>(pair.first), pair.second};

    default:
      apache::thrift::rocket::detail::throwUnexpectedFrameType(pair.first);
  }
}

ExtFrameType readExtFrameType(folly::io::Cursor& cursor) {
  cursor.readBE<uint32_t>();
  return ExtFrameType::UNKNOWN;
}

// Has both false positives and false negatives
bool isMaybeRocketFrame(const folly::IOBuf& data) {
  if (data.length() < Serializer::kBytesForFrameOrMetadataLength) {
    return false;
  }

  try {
    folly::io::Cursor cursor(&data);
    auto size = readFrameOrMetadataSize(cursor);
    if (size + Serializer::kBytesForFrameOrMetadataLength != data.length()) {
      return false;
    }
    const size_t kMinFrameSize = 6;
    if (size < kMinFrameSize) {
      return false;
    }

    auto streamId = static_cast<uint32_t>(readStreamId(cursor));
    if (streamId != 0 && streamId % 2 == 0) {
      // Thrift only uses client -> server streams
      return false;
    }

    const uint16_t frameTypeAndFlags = cursor.readBE<uint16_t>();
    const uint16_t flags = frameTypeAndFlags & Flags::kMask;
    const uint16_t kMinFlag = 1 << 5;
    if (flags != 0 && flags < kMinFlag) {
      return false;
    }

    const uint8_t frameType = frameTypeAndFlags >> Flags::kBits;
    switch (static_cast<FrameType>(frameType)) {
      case FrameType::SETUP:
      case FrameType::KEEPALIVE:
      case FrameType::METADATA_PUSH:
        return streamId == 0;
      case FrameType::REQUEST_RESPONSE:
      case FrameType::REQUEST_FNF:
      case FrameType::REQUEST_STREAM:
      case FrameType::REQUEST_CHANNEL:
      case FrameType::REQUEST_N:
      case FrameType::CANCEL:
      case FrameType::PAYLOAD:
        return streamId != 0;
      case FrameType::ERROR:
      case FrameType::EXT:
        return true;
      default:
        return false;
    }
  } catch (...) {
    return false;
  }
}
} // namespace rocket
} // namespace thrift
} // namespace apache
