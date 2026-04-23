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

#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>

#include <thrift/lib/cpp2/fast_thrift/frame/FrameDescriptor.h>

#include <folly/io/Cursor.h>
#include <folly/io/IOBufQueue.h>
#include <folly/lang/Bits.h>

#include <array>
#include <cstring>

namespace apache::thrift::fast_thrift::frame::write {

using namespace apache::thrift::fast_thrift::frame;

namespace detail {

using namespace apache::thrift::fast_thrift::frame::detail;

// Write 3-byte big-endian size (for frame length and metadata length fields)
void writeMetadataLengthBE(folly::io::QueueAppender& appender, uint32_t value) {
  std::array<uint8_t, kMetadataLengthSize> bytes = {
      static_cast<uint8_t>(value >> 16),
      static_cast<uint8_t>(value >> 8),
      static_cast<uint8_t>(value)};
  appender.push(bytes.data(), bytes.size());
}

// Helper to write big-endian integers to byte arrays using folly utilities
template <typename T>
void writeBE(uint8_t* dest, T value) {
  folly::storeUnaligned<T>(dest, folly::Endian::big(value));
}

// Write type + flags as 2-byte big-endian
void writeTypeAndFlags(
    folly::io::QueueAppender& appender, FrameType type, uint16_t flags) {
  uint16_t typeAndFlags =
      (static_cast<uint16_t>(type) << kFlagsBits) | (flags & kFlagsMask);
  appender.writeBE<uint16_t>(typeAndFlags);
}

// Write 3-byte metadata length directly into a byte buffer
void writeMetadataLengthBE(uint8_t* dest, uint32_t value) {
  dest[0] = static_cast<uint8_t>(value >> 16);
  dest[1] = static_cast<uint8_t>(value >> 8);
  dest[2] = static_cast<uint8_t>(value);
}

// Write frame header directly into the metadata buffer's headroom.
// Returns the metadata buffer with the header prepended (zero allocation).
std::unique_ptr<folly::IOBuf> serializeFrameIntoHeadroom(
    FrameType type,
    uint32_t streamId,
    uint16_t flags,
    const uint8_t* extraHeader,
    size_t extraHeaderSize,
    size_t headerSize,
    size_t metadataLen,
    std::unique_ptr<folly::IOBuf> metadata,
    std::unique_ptr<folly::IOBuf> data) {
  // Write the header backwards from the metadata data pointer.
  // Layout: [streamId(4)][typeFlags(2)][extraHeader(N)][metaLen(3)]
  uint8_t* headerStart = metadata->writableData() - headerSize;

  size_t offset = 0;

  // Stream ID (4 bytes BE)
  writeBE(headerStart + offset, streamId);
  offset += sizeof(uint32_t);

  // Type + Flags (2 bytes BE)
  uint16_t typeAndFlags =
      (static_cast<uint16_t>(type) << kFlagsBits) | (flags & kFlagsMask);
  writeBE(headerStart + offset, typeAndFlags);
  offset += sizeof(uint16_t);

  // Frame-specific extra header bytes
  if (extraHeaderSize > 0 && extraHeader != nullptr) {
    std::memcpy(headerStart + offset, extraHeader, extraHeaderSize);
    offset += extraHeaderSize;
  }

  // Metadata length (3 bytes BE)
  writeMetadataLengthBE(
      headerStart + offset, static_cast<uint32_t>(metadataLen));

  // Prepend to expose the header bytes
  metadata->prepend(headerSize);

  // Chain data after metadata
  if (data != nullptr && data->computeChainDataLength() > 0) {
    metadata->appendChain(std::move(data));
  }

  return metadata;
}

// Core serialization function - allocates header, chains metadata/data
std::unique_ptr<folly::IOBuf> serializeFrame(
    FrameType type,
    uint32_t streamId,
    uint16_t flags,
    const uint8_t* extraHeader,
    size_t extraHeaderSize,
    std::unique_ptr<folly::IOBuf> metadata,
    std::unique_ptr<folly::IOBuf> data) {
  bool hasMetadata =
      metadata != nullptr && metadata->computeChainDataLength() > 0;

  if (hasMetadata) {
    flags |= kMetadataBit;
  }

  size_t headerSize = kBaseHeaderSize + extraHeaderSize;
  if (hasMetadata) {
    headerSize += kMetadataLengthSize;
  }

  // Fast path: write frame header into the metadata buffer's headroom.
  // Conditions: has metadata, not chained (single buffer), not shared,
  // and sufficient headroom for the entire frame header.
  if (hasMetadata && !metadata->isChained() && !metadata->isSharedOne() &&
      metadata->headroom() >= headerSize) {
    size_t metadataLen = metadata->length();
    return serializeFrameIntoHeadroom(
        type,
        streamId,
        flags,
        extraHeader,
        extraHeaderSize,
        headerSize,
        metadataLen,
        std::move(metadata),
        std::move(data));
  }

  // Slow path: allocate a new header buffer.
  folly::IOBufQueue queue;
  queue.append(folly::IOBuf::create(headerSize));
  folly::io::QueueAppender appender(&queue, 0);

  // Stream ID (4 bytes)
  appender.writeBE<uint32_t>(streamId);

  // Type + Flags (2 bytes)
  writeTypeAndFlags(appender, type, flags);

  // Frame-specific header bytes
  if (extraHeaderSize > 0 && extraHeader != nullptr) {
    appender.push(extraHeader, extraHeaderSize);
  }

  // Metadata length (3 bytes) if present
  if (hasMetadata) {
    auto metaLen = metadata->computeChainDataLength();
    writeMetadataLengthBE(appender, static_cast<uint32_t>(metaLen));
  }

  // Chain metadata (take ownership, no cloning)
  if (hasMetadata) {
    queue.append(std::move(metadata));
  }

  // Chain data (take ownership, no cloning)
  if (data != nullptr && data->computeChainDataLength() > 0) {
    queue.append(std::move(data));
  }

  return queue.move();
}

} // namespace detail

// ============================================================================
// REQUEST_RESPONSE
// ============================================================================

std::unique_ptr<folly::IOBuf> serialize(
    const RequestResponseHeader& header,
    std::unique_ptr<folly::IOBuf> metadata,
    std::unique_ptr<folly::IOBuf> data) {
  uint16_t flags = 0;
  if (header.follows) {
    flags |= detail::kFollowsBit;
  }

  return detail::serializeFrame(
      FrameType::REQUEST_RESPONSE,
      header.streamId,
      flags,
      nullptr,
      0,
      std::move(metadata),
      std::move(data));
}

// ============================================================================
// REQUEST_FNF
// ============================================================================

std::unique_ptr<folly::IOBuf> serialize(
    const RequestFnfHeader& header,
    std::unique_ptr<folly::IOBuf> metadata,
    std::unique_ptr<folly::IOBuf> data) {
  uint16_t flags = 0;
  if (header.follows) {
    flags |= detail::kFollowsBit;
  }

  return detail::serializeFrame(
      FrameType::REQUEST_FNF,
      header.streamId,
      flags,
      nullptr,
      0,
      std::move(metadata),
      std::move(data));
}

// ============================================================================
// REQUEST_STREAM
// ============================================================================

std::unique_ptr<folly::IOBuf> serialize(
    const RequestStreamHeader& header,
    std::unique_ptr<folly::IOBuf> metadata,
    std::unique_ptr<folly::IOBuf> data) {
  uint16_t flags = 0;
  if (header.follows) {
    flags |= detail::kFollowsBit;
  }

  // Extra header: initialRequestN (4 bytes big-endian)
  std::array<uint8_t, detail::kRequestNHeaderSize - kBaseHeaderSize> extra{};
  detail::writeBE(extra.data(), header.initialRequestN);

  return detail::serializeFrame(
      FrameType::REQUEST_STREAM,
      header.streamId,
      flags,
      extra.data(),
      extra.size(),
      std::move(metadata),
      std::move(data));
}

// ============================================================================
// REQUEST_CHANNEL
// ============================================================================

std::unique_ptr<folly::IOBuf> serialize(
    const RequestChannelHeader& header,
    std::unique_ptr<folly::IOBuf> metadata,
    std::unique_ptr<folly::IOBuf> data) {
  uint16_t flags = 0;
  if (header.follows) {
    flags |= detail::kFollowsBit;
  }
  if (header.complete) {
    flags |= detail::kCompleteBit;
  }

  std::array<uint8_t, detail::kRequestNHeaderSize - kBaseHeaderSize> extra{};
  detail::writeBE(extra.data(), header.initialRequestN);

  return detail::serializeFrame(
      FrameType::REQUEST_CHANNEL,
      header.streamId,
      flags,
      extra.data(),
      extra.size(),
      std::move(metadata),
      std::move(data));
}

// ============================================================================
// REQUEST_N
// ============================================================================

std::unique_ptr<folly::IOBuf> serialize(const RequestNHeader& header) {
  std::array<uint8_t, detail::kRequestNHeaderSize - kBaseHeaderSize> extra{};
  detail::writeBE(extra.data(), header.requestN);

  return detail::serializeFrame(
      FrameType::REQUEST_N,
      header.streamId,
      0, // no flags
      extra.data(),
      extra.size(),
      nullptr,
      nullptr);
}

// ============================================================================
// CANCEL
// ============================================================================

std::unique_ptr<folly::IOBuf> serialize(const CancelHeader& header) {
  return detail::serializeFrame(
      FrameType::CANCEL,
      header.streamId,
      0, // no flags
      nullptr,
      0,
      nullptr,
      nullptr);
}

// ============================================================================
// PAYLOAD
// ============================================================================

std::unique_ptr<folly::IOBuf> serialize(
    const PayloadHeader& header,
    std::unique_ptr<folly::IOBuf> metadata,
    std::unique_ptr<folly::IOBuf> data) {
  uint16_t flags = 0;
  if (header.follows) {
    flags |= detail::kFollowsBit;
  }
  if (header.complete) {
    flags |= detail::kCompleteBit;
  }
  if (header.next) {
    flags |= detail::kNextBit;
  }

  return detail::serializeFrame(
      FrameType::PAYLOAD,
      header.streamId,
      flags,
      nullptr,
      0,
      std::move(metadata),
      std::move(data));
}

// ============================================================================
// ERROR
// ============================================================================

std::unique_ptr<folly::IOBuf> serialize(
    const ErrorHeader& header,
    std::unique_ptr<folly::IOBuf> metadata,
    std::unique_ptr<folly::IOBuf> data) {
  std::array<uint8_t, detail::kErrorHeaderSize - kBaseHeaderSize> extra{};
  detail::writeBE(extra.data(), header.errorCode);

  return detail::serializeFrame(
      FrameType::ERROR,
      header.streamId,
      0, // no flags
      extra.data(),
      extra.size(),
      std::move(metadata),
      std::move(data));
}

// ============================================================================
// KEEPALIVE
// ============================================================================

std::unique_ptr<folly::IOBuf> serialize(
    const KeepAliveHeader& header, std::unique_ptr<folly::IOBuf> data) {
  uint16_t flags = 0;
  if (header.respond) {
    flags |= detail::kRespondBit;
  }

  std::array<uint8_t, detail::kKeepAliveHeaderSize - kBaseHeaderSize> extra{};
  detail::writeBE(extra.data(), header.lastReceivedPosition);

  return detail::serializeFrame(
      FrameType::KEEPALIVE,
      0, // Stream ID always 0 for KEEPALIVE
      flags,
      extra.data(),
      extra.size(),
      nullptr,
      std::move(data));
}

// ============================================================================
// SETUP
// ============================================================================

std::unique_ptr<folly::IOBuf> serialize(
    const SetupHeader& header,
    std::unique_ptr<folly::IOBuf> metadata,
    std::unique_ptr<folly::IOBuf> data) {
  uint16_t flags = 0;
  if (header.lease) {
    flags |= detail::kLeaseBit;
  }

  static constexpr std::string_view kRocketMetadataBinaryMimeType{
      "application/x-rocket-metadata+binary"};
  static constexpr std::string_view kRocketPayloadMimeType{
      "application/x-rocket-payload"};

  // Extra header layout (R flag not set, so no resume token fields):
  // - Major version: 2 bytes (offset 0)
  // - Minor version: 2 bytes (offset 2)
  // - Keepalive time: 4 bytes (offset 4)
  // - Max lifetime: 4 bytes (offset 8)
  // - Metadata MIME size: 1 byte (offset 12)
  // - Metadata MIME type: 37 bytes (offset 13)
  // - Data MIME size: 1 byte (offset 50)
  // - Data MIME type: 28 bytes (offset 51)
  constexpr size_t kSetupExtraHeaderBaseSize =
      12; // version(4) + keepalive(4) + lifetime(4)
  constexpr size_t extraSize = kSetupExtraHeaderBaseSize + sizeof(uint8_t) +
      kRocketMetadataBinaryMimeType.size() + sizeof(uint8_t) +
      kRocketPayloadMimeType.size();
  std::array<uint8_t, extraSize> extra;

  detail::writeBE(&extra[0], header.majorVersion);
  detail::writeBE(&extra[2], header.minorVersion);
  detail::writeBE(&extra[4], header.keepaliveTime);
  detail::writeBE(&extra[8], header.maxLifetime);
  // Note: Resume token length field is NOT written when R flag is not set

  // Metadata MIME type (length + string)
  constexpr size_t metadataMimeOffset = 12;
  extra[metadataMimeOffset] =
      static_cast<uint8_t>(kRocketMetadataBinaryMimeType.size());
  std::memcpy(
      &extra[metadataMimeOffset + 1],
      kRocketMetadataBinaryMimeType.data(),
      kRocketMetadataBinaryMimeType.size());

  // Data MIME type (length + string)
  const size_t dataMimeOffset =
      metadataMimeOffset + 1 + kRocketMetadataBinaryMimeType.size();
  extra[dataMimeOffset] = static_cast<uint8_t>(kRocketPayloadMimeType.size());
  std::memcpy(
      &extra[dataMimeOffset + 1],
      kRocketPayloadMimeType.data(),
      kRocketPayloadMimeType.size());

  return detail::serializeFrame(
      FrameType::SETUP,
      0, // Stream ID always 0 for SETUP
      flags,
      extra.data(),
      extra.size(),
      std::move(metadata),
      std::move(data));
}

// ============================================================================
// METADATA_PUSH
// ============================================================================

std::unique_ptr<folly::IOBuf> serialize(
    const MetadataPushHeader& /* header */,
    std::unique_ptr<folly::IOBuf> metadata) {
  // METADATA_PUSH is special - the entire payload is metadata
  // M flag is always set per RSocket spec (no 3-byte metadata length prefix)

  folly::IOBufQueue queue;
  queue.append(folly::IOBuf::create(kBaseHeaderSize));
  folly::io::QueueAppender appender(&queue, 0);

  appender.writeBE<uint32_t>(0); // Stream ID always 0
  detail::writeTypeAndFlags(
      appender, FrameType::METADATA_PUSH, detail::kMetadataBit);

  if (metadata != nullptr) {
    queue.append(std::move(metadata));
  }

  return queue.move();
}

// ============================================================================
// EXT
// ============================================================================

std::unique_ptr<folly::IOBuf> serialize(
    const ExtHeader& header,
    std::unique_ptr<folly::IOBuf> metadata,
    std::unique_ptr<folly::IOBuf> data) {
  uint16_t flags = 0;
  if (header.ignore) {
    flags |= detail::kIgnoreBit;
  }

  std::array<uint8_t, detail::kExtHeaderSize - kBaseHeaderSize> extra{};
  detail::writeBE(extra.data(), header.extendedType);

  return detail::serializeFrame(
      FrameType::EXT,
      header.streamId,
      flags,
      extra.data(),
      extra.size(),
      std::move(metadata),
      std::move(data));
}

} // namespace apache::thrift::fast_thrift::frame::write
