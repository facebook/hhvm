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

namespace {
constexpr size_t kPageSize = 4096;
} // namespace

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
  const auto extFrameType = cursor.readBE<uint32_t>();
  switch (static_cast<ExtFrameType>(extFrameType)) {
    case ExtFrameType::ALIGNED_PAGE:
    case ExtFrameType::CUSTOM_ALLOC:
      return static_cast<ExtFrameType>(extFrameType);
    default:
      return ExtFrameType::UNKNOWN;
  }
}

bool alignTo4k(folly::IOBuf& buffer, size_t startOffset, size_t frameSize) {
  auto iobuf = get4kAlignedBuf(
      std::max(buffer.length(), frameSize), startOffset, buffer.length());
  if (UNLIKELY(!iobuf)) {
    return false;
  }

  memcpy(iobuf->writableData(), buffer.writableData(), buffer.length());
  buffer = *std::move(iobuf);
  return true;
}

bool alignTo4kBufQueue(
    folly::IOBufQueue& bufQueue, size_t startOffset, size_t frameSize) {
  auto iobuf = get4kAlignedBuf(
      std::max(bufQueue.chainLength(), frameSize),
      startOffset,
      bufQueue.chainLength());
  if (UNLIKELY(!iobuf)) {
    return false;
  }

  folly::io::Cursor cursor(bufQueue.front());
  cursor.pull(iobuf->writableData(), bufQueue.chainLength());
  folly::IOBufQueue bufQ{folly::IOBufQueue::cacheChainLength()};
  bufQ.append(*std::move(iobuf));
  bufQueue = std::move(bufQ);
  return true;
}

std::unique_ptr<folly::IOBuf> get4kAlignedBuf(
    size_t numBytes, size_t startOffset, size_t trimLength) {
  DCHECK_LE(trimLength, numBytes);
  const size_t padding = kPageSize - (startOffset % kPageSize);
  const size_t size = numBytes + padding;
  void* rawbuf = folly::aligned_malloc(size, kPageSize);
  if (UNLIKELY(!rawbuf)) {
    LOG(ERROR) << "Allocating : " << kPageSize
               << " aligned memory of size: " << numBytes << " failed!";
    return nullptr;
  }
  auto iobuf = folly::IOBuf::takeOwnership(
      rawbuf, size, size, [](void* p, void*) { folly::aligned_free(p); });
  iobuf->trimStart(padding);
  iobuf->trimEnd(size - std::min(numBytes, trimLength) - padding);
  return iobuf;
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
